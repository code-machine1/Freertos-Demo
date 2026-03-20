/**
 * @file wifi_uart.c
 * @brief WiFi模块底层驱动实现（使用UART4直接发送，适配AT32库函数）
 */

#include "wifi_uart.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include <string.h>
#include <stdio.h>
/* AT32 头文件 */
#include "at32f403a_407.h"
#include "at32f403a_407_usart.h"

#include "EventRecorder.h"
/* 声明外部队列句柄（由Workbench生成） */
extern QueueHandle_t uart4_queue_handle;

/* 响应解析状态机 */
typedef enum
{
    STATE_IDLE = 0,
    STATE_O,
    STATE_OK,
    STATE_E,
    STATE_ER,
    STATE_ERR,
    STATE_ERRO,
    STATE_ERROR
} at_response_state_t;

/* 静态函数声明 */
static int check_at_response(at_response_state_t *state, char ch);
static void uart4_send_bytes(const uint8_t *data, uint32_t len);

/**
 * @brief 通过UART4发送多个字节（阻塞发送，每个字节前等待TDBE）
 * @param data 数据指针
 * @param len  字节数
 */
static void uart4_send_bytes(const uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        while (usart_flag_get(UART4, USART_TDBE_FLAG) == RESET)
            ;
        usart_data_transmit(UART4, data[i]);
    }
    // 等待最后一个字节发送完成（移位寄存器空）
    while (usart_flag_get(UART4, USART_TDC_FLAG) == RESET)
        ;
}

/*------------------ 接口实现 ------------------*/

int wifi_uart_init(void)
{
    /* 检查外部队列是否已创建 */
    if (uart4_queue_handle == NULL)
    {
        return -1;
    }
    /* 无需其他初始化，串口硬件已由Workbench配置好 */
    return 0;
}

void wifi_uart_rx_isr(uint8_t data)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (uart4_queue_handle != NULL)
    {
        xQueueSendFromISR(uart4_queue_handle, &data, &xHigherPriorityTaskWoken);
        //printf("RX: 0x%02X (%c)\r\n", data, (data >= 0x20 && data <= 0x7E) ? data : '.');
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void wifi_send_raw(const uint8_t *data, uint32_t len)
{
    if (data != NULL && len > 0)
    {
        uart4_send_bytes(data, len);
    }
}

int wifi_recv_raw(uint8_t *buf, uint32_t len, uint32_t timeout_ms)
{
    uint32_t received = 0;
    TickType_t xTicksToWait = pdMS_TO_TICKS(timeout_ms);
    TickType_t xStartTick = xTaskGetTickCount();

    while (received < len)
    {
        TickType_t xRemaining = xStartTick + xTicksToWait - xTaskGetTickCount();
        if (xRemaining <= 0)
            break;

        if (xQueueReceive(uart4_queue_handle, &buf[received], xRemaining) == pdTRUE)
        {
            received++;
        }
        else
        {
            break;
        }
    }
    return received;
}

void wifi_uart_clear_rx(void)
{
    uint8_t dummy;
    while (xQueueReceive(uart4_queue_handle, &dummy, 0) == pdTRUE)
        ;
}

int wifi_send_at_cmd(const char *cmd, char *response, uint32_t resp_len, uint32_t timeout_ms)
{
    if (cmd == NULL)
        return -1;
    // 临时禁用中断，清空硬件RX FIFO
    NVIC_DisableIRQ(UART4_IRQn);
    while (usart_flag_get(UART4, USART_RDBF_FLAG) == SET)
    {
        (void)usart_data_receive(UART4);
    }
    NVIC_ClearPendingIRQ(UART4_IRQn);
    NVIC_EnableIRQ(UART4_IRQn);

     /* 清空接收队列，避免历史数据干扰 */
    wifi_uart_clear_rx();

    /* 发送命令（自动添加 \r\n） */
    uint32_t cmd_len = strlen(cmd);
    uint8_t *send_buf = pvPortMalloc(cmd_len + 2);
    if (send_buf == NULL)
        return -1;

    memcpy(send_buf, cmd, cmd_len);
    send_buf[cmd_len] = '\r';
    send_buf[cmd_len + 1] = '\n';
    uart4_send_bytes(send_buf, cmd_len + 2);
    vPortFree(send_buf);

    /* 接收并解析响应 */
    at_response_state_t state = STATE_IDLE;
    TickType_t xStartTick = xTaskGetTickCount();
    TickType_t xTicksToWait = pdMS_TO_TICKS(timeout_ms);
    uint32_t resp_idx = 0;
    int result = -1;

    while (1)
    {
        TickType_t xRemaining = xStartTick + xTicksToWait - xTaskGetTickCount();
        if (xRemaining <= 0)
            break;

        uint8_t ch;
        if (xQueueReceive(uart4_queue_handle, &ch, xRemaining) != pdTRUE)
        {
            break;
        }

        /* 保存响应（如果提供了缓冲区） */
        if (response != NULL && resp_idx < resp_len - 1)
        {
            response[resp_idx++] = (char)ch;
            response[resp_idx] = '\0';
        }

        int ret = check_at_response(&state, (char)ch);
        if (ret == 1)
        {
            result = 0; /* 收到OK */
            break;
        }
        else if (ret == 2)
        {
            result = 1; /* 收到ERROR */
            break;
        }
    }

    return result;
}

/**
 * @brief 检查AT响应状态机（辅助函数）
 */
static int check_at_response(at_response_state_t *state, char ch)
{
    switch (*state)
    {
    case STATE_IDLE:
        if (ch == 'O')
            *state = STATE_O;
        else if (ch == 'E')
            *state = STATE_E;
        break;
    case STATE_O:
        if (ch == 'K')
        {
            *state = STATE_OK;
            return 1;
        }
        else
        {
            *state = STATE_IDLE;
        }
        break;
    case STATE_E:
        if (ch == 'R')
            *state = STATE_ER;
        else
            *state = STATE_IDLE;
        break;
    case STATE_ER:
        if (ch == 'R')
            *state = STATE_ERR;
        else
            *state = STATE_IDLE;
        break;
    case STATE_ERR:
        if (ch == 'O')
            *state = STATE_ERRO;
        else
            *state = STATE_IDLE;
        break;
    case STATE_ERRO:
        if (ch == 'R')
        {
            *state = STATE_ERROR;
            return 2;
        }
        else
        {
            *state = STATE_IDLE;
        }
        break;
    default:
        *state = STATE_IDLE;
        break;
    }
    return 0;
}