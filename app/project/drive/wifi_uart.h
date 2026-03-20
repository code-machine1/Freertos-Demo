/**
 * @file wifi_uart.h
 * @brief WiFi模块底层串口驱动（基于FreeRTOS，使用UART4）
 */

#ifndef WIFI_UART_H
#define WIFI_UART_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief 初始化WiFi底层驱动（检查队列）
     * @return 0成功，-1失败（队列句柄无效）
     */
    int wifi_uart_init(void);

    /**
     * @brief 串口接收中断处理函数，由用户在UART4中断中调用
     * @param data 接收到的字节
     */
    void wifi_uart_rx_isr(uint8_t data);

    /**
     * @brief 发送原始数据（不等待响应）
     * @param data 数据指针
     * @param len 数据长度
     */
    void wifi_send_raw(const uint8_t *data, uint32_t len);

    /**
     * @brief 接收原始数据（从队列中读取）
     * @param buf 缓冲区
     * @param len 期望接收的字节数
     * @param timeout_ms 超时时间（毫秒）
     * @return 实际接收到的字节数
     */
    int wifi_recv_raw(uint8_t *buf, uint32_t len, uint32_t timeout_ms);

    /**
     * @brief 清空接收队列（丢弃所有未读数据）
     */
    void wifi_uart_clear_rx(void);

    /**
     * @brief 发送AT命令并等待响应（自动添加\r\n）
     * @param cmd 命令字符串（不含\r\n）
     * @param response 存储完整响应的缓冲区（可为NULL）
     * @param resp_len 缓冲区大小
     * @param timeout_ms 超时时间（毫秒）
     * @return 0=收到OK, 1=收到ERROR, -1=超时或错误
     */
    int wifi_send_at_cmd(const char *cmd, char *response, uint32_t resp_len, uint32_t timeout_ms);
    
#ifdef __cplusplus
}
#endif

#endif /* WIFI_UART_H */