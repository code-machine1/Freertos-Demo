/* add user code begin Header */
/**
 ******************************************************************************
 * File Name          : freertos_app.c
 * Description        : Code for freertos applications
 */
/* add user code end Header */

/* Includes ------------------------------------------------------------------*/
#include "freertos_app.h"

/* private includes ----------------------------------------------------------*/
/* add user code begin private includes */

/* add user code end private includes */

/* private typedef -----------------------------------------------------------*/
/* add user code begin private typedef */

/* add user code end private typedef */

/* private define ------------------------------------------------------------*/
/* add user code begin private define */

/* add user code end private define */

/* private macro -------------------------------------------------------------*/
/* add user code begin private macro */

/* add user code end private macro */

/* private variables ---------------------------------------------------------*/
/* add user code begin private variables */

/* add user code end private variables */

/* private function prototypes --------------------------------------------*/
/* add user code begin function prototypes */
void breath_led(gpio_type *gpio_x, uint16_t pin);
void usart1_send_string(const char *str);
void usart1_send_uint32(uint32_t num);
void usart1_send_hex32(uint32_t num);
static void usart1_send_hex_buffer(uint8_t *buf, uint32_t len);
/* add user code end function prototypes */

/* private user code ---------------------------------------------------------*/
/* add user code begin 0 */

/* add user code end 0 */

/* task handler */
TaskHandle_t lcd_task_handle;
TaskHandle_t key_task_handle;
TaskHandle_t iap_task_handle;
TaskHandle_t beep_task_handle;
TaskHandle_t wdt_task_handle;
TaskHandle_t led_task_handle;
TaskHandle_t pc_task_handle;
TaskHandle_t wifi_task_handle;
TaskHandle_t flash_task_handle;

/* queue handler */
QueueHandle_t usart1_queue_handle;
QueueHandle_t uart4_queue_handle;

/* Idle task control block and stack */
static StackType_t idle_task_stack[configMINIMAL_STACK_SIZE];
static StackType_t timer_task_stack[configTIMER_TASK_STACK_DEPTH];

static StaticTask_t idle_task_tcb;
static StaticTask_t timer_task_tcb;

/* External Idle and Timer task static memory allocation functions */
extern void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );
extern void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize );

/*
  vApplicationGetIdleTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetIdleTaskMemory( StaticTask_t ** ppxIdleTaskTCBBuffer, StackType_t ** ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &idle_task_tcb;
  *ppxIdleTaskStackBuffer = &idle_task_stack[0];
  *pulIdleTaskStackSize = (uint32_t)configMINIMAL_STACK_SIZE;
}
/*
  vApplicationGetTimerTaskMemory gets called when configSUPPORT_STATIC_ALLOCATION
  equals to 1 and is required for static memory allocation support.
*/
void vApplicationGetTimerTaskMemory( StaticTask_t ** ppxTimerTaskTCBBuffer, StackType_t ** ppxTimerTaskStackBuffer, uint32_t * pulTimerTaskStackSize )
{
  *ppxTimerTaskTCBBuffer = &timer_task_tcb;
  *ppxTimerTaskStackBuffer = &timer_task_stack[0];
  *pulTimerTaskStackSize = (uint32_t)configTIMER_TASK_STACK_DEPTH;
}

/* add user code begin 1 */

/* add user code end 1 */

/**
  * @brief  initializes all task.
  * @param  none
  * @retval none
  */
void freertos_task_create(void)
{
  /* create lcd_task task */
  xTaskCreate(lcd_function,
              "lcd_task",
              128,
              NULL,
              0,
              &lcd_task_handle);

  /* create key_task task */
  xTaskCreate(key_function,
              "key_task",
              128,
              NULL,
              0,
              &key_task_handle);

  /* create iap_task task */
  xTaskCreate(iap_function,
              "iap_task",
              128,
              NULL,
              0,
              &iap_task_handle);

  /* create beep_task task */
  xTaskCreate(beep_function,
              "beep_task",
              128,
              NULL,
              0,
              &beep_task_handle);

  /* create wdt_task task */
  xTaskCreate(wdt_function,
              "wdt_task",
              128,
              NULL,
              0,
              &wdt_task_handle);

  /* create led_task task */
  xTaskCreate(led_function,
              "led_task",
              128,
              NULL,
              0,
              &led_task_handle);

  /* create pc_task task */
  xTaskCreate(pc_function,
              "pc_task",
              128,
              NULL,
              0,
              &pc_task_handle);

  /* create wifi_task task */
  xTaskCreate(wifi_function,
              "wifi_task",
              4096,
              NULL,
              0,
              &wifi_task_handle);

  /* create flash_task task */
  xTaskCreate(flash_function,
              "flash_task",
              2048,
              NULL,
              0,
              &flash_task_handle);
}

/**
  * @brief  initializes all queue.
  * @param  none
  * @retval none
  */
void freertos_queue_create(void)
{
  /* Create the usart1_queue, storing the returned handle in the xQueue variable. */
  usart1_queue_handle = xQueueCreate(1024, sizeof(uint8_t));

  /* Create the uart4_queue, storing the returned handle in the xQueue variable. */
  uart4_queue_handle = xQueueCreate(1024, sizeof(uint8_t));
}

/**
  * @brief  freertos init and begin run.
  * @param  none
  * @retval none
  */
void wk_freertos_init(void)
{
  /* add user code begin freertos_init 0 */
    /* add user code end freertos_init 0 */

  /* enter critical */
  taskENTER_CRITICAL();

  freertos_queue_create();
  freertos_task_create();
	
  /* add user code begin freertos_init 1 */
    /* add user code end freertos_init 1 */

  /* exit critical */
  taskEXIT_CRITICAL();

  /* start scheduler */
  vTaskStartScheduler();
}

/**
  * @brief lcd_task function.
  * @param  none
  * @retval none
  */
void lcd_function(void *pvParameters)
{
  /* add user code begin lcd_function 0 */
    lcd_init();
    usr_touchInit();
    /* add user code end lcd_function 0 */

  /* add user code begin lcd_function 2 */
    lcd_clear(RED);
    /* add user code end lcd_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin lcd_function 1 */
        vTaskDelay(1000);
        /* add user code end lcd_function 1 */
  }
}


/**
  * @brief key_task function.
  * @param  none
  * @retval none
  */
void key_function(void *pvParameters)
{
  /* add user code begin key_function 0 */

    /* add user code end key_function 0 */

  /* add user code begin key_function 2 */

    /* add user code end key_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin key_function 1 */
        key_handle();
        vTaskDelay(KEY_TASK_TIME);
        /* add user code end key_function 1 */
  }
}


/**
  * @brief iap_task function.
  * @param  none
  * @retval none
  */
void iap_function(void *pvParameters)
{
  /* add user code begin iap_function 0 */
    iap_init();

    /* add user code end iap_function 0 */

  /* add user code begin iap_function 2 */

    /* add user code end iap_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin iap_function 1 */
        iap_command_handle();
        vTaskDelay(10);
        /* add user code end iap_function 1 */
  }
}


/**
  * @brief beep_task function.
  * @param  none
  * @retval none
  */
void beep_function(void *pvParameters)
{
  /* add user code begin beep_function 0 */

    /* add user code end beep_function 0 */

  /* add user code begin beep_function 2 */

    /* add user code end beep_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin beep_function 1 */
        vTaskDelay(1);
        /* add user code end beep_function 1 */
  }
}


/**
  * @brief wdt_task function.
  * @param  none
  * @retval none
  */
void wdt_function(void *pvParameters)
{
  /* add user code begin wdt_function 0 */

    /* add user code end wdt_function 0 */

  /* add user code begin wdt_function 2 */
    // wdt_enable();
    /* add user code end wdt_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin wdt_function 1 */
        wdt_counter_reload();
        vTaskDelay(200);
        /* add user code end wdt_function 1 */
  }
}


/**
  * @brief led_task function.
  * @param  none
  * @retval none
  */
void led_function(void *pvParameters)
{
  /* add user code begin led_function 0 */

    /* add user code end led_function 0 */

  /* add user code begin led_function 2 */

    /* add user code end led_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin led_function 1 */
        breath_led(GPIOA, GPIO_PINS_12);
        vTaskDelay(1);
        /* add user code end led_function 1 */
  }
}


/**
  * @brief pc_task function.
  * @param  none
  * @retval none
  */
void pc_function(void *pvParameters)
{
  /* add user code begin pc_function 0 */
    uint8_t send_buff[] = {"uasrt send"};
    /* add user code end pc_function 0 */

  /* add user code begin pc_function 2 */

    /* add user code end pc_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin pc_function 1 */
        vTaskDelay(10);
        /* add user code end pc_function 1 */
  }
}


/**
  * @brief wifi_task function.
  * @param  none
  * @retval none
  */
void wifi_function(void *pvParameters)
{
  /* add user code begin wifi_function 0 */
    uint8_t test_step = 0;
    /* add user code end wifi_function 0 */

  /* add user code begin wifi_function 2 */
    vTaskDelay(2000);
    wifi_handle_init();
    vTaskDelay(100);
    wifi_provision_start("WiFi_Config", "12345678", 120); // 热点SSID和密码，超时120秒
    vTaskDelay(2000);
    /* add user code end wifi_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin wifi_function 1 */
        wifi_provision_process();
        provision_state_t state = wifi_provision_get_state();

        if (state == PROV_STATE_CONNECTED)
        {
            provision_result_t result;

            if (wifi_provision_get_result(&result) == 0)
            {
                printf("Network configuration successful,SSID: %s, IP: %s\n", result.ssid, result.ip);
                // 可选：停止配网，进入正常模式
                wifi_provision_stop();
            }
        }
        else if (state == PROV_STATE_TIMEOUT || state == PROV_STATE_IDLE)
        {
            printf("Network configuration failed or timed out\n");
            // 可在此处重启配网
            vTaskDelay(5000);
            wifi_provision_start("WiFi_Config", "12345678", 120);
        }

        vTaskDelay(30);
        /* add user code end wifi_function 1 */
  }
}


/**
  * @brief flash_task function.
  * @param  none
  * @retval none
  */
void flash_function(void *pvParameters)
{
  /* add user code begin flash_function 0 */
  extern QueueHandle_t usart1_queue_handle;
    uint8_t rx_byte;

    typedef enum {
        WAIT_START,
        WAIT_FIRST_BYTE,   // 等待第一个数据字节（较长超时）
        RECEIVING          // 正在接收数据块（较长超时，允许手动发送结束符）
    } state_t;
    state_t state = WAIT_START;
 
  /* add user code end flash_function 0 */

  /* add user code begin flash_function 2 */
uint8_t buffer[256];
    uint32_t buf_len = 0;
    uint32_t write_addr = 0;
    uint32_t total_written = 0;

    const uint8_t start_pattern[] = {'f','l','a','s','h',' ','s','t','a','r','t'};
    uint8_t start_idx = 0;

    const uint8_t end_pattern[] = {'f','l','a','s','h',' ','e','n','d'};
    #define END_LEN 9
    uint8_t end_window[END_LEN] = {0};
    uint8_t end_pos = 0;
	    if (usart1_queue_handle == NULL) {
        usart1_send_string("ERROR: queue is NULL\r\n");
        while(1);
    } else {
        usart1_send_string("Flash task ready\r\n");
    }
  /* add user code end flash_function 2 */

  /* Infinite loop */
  while(1)
  {
  /* add user code begin flash_function 1 */
       switch (state)
        {
            case WAIT_START:
                /* 阻塞等待 "flash start" */
                if (xQueueReceive(usart1_queue_handle, &rx_byte, portMAX_DELAY) == pdTRUE)
                {
                    if (rx_byte == start_pattern[start_idx]) {
                        start_idx++;
                        if (start_idx == 11) {
                            /* 清空队列，进入等待第一个字节状态 */
                            xQueueReset(usart1_queue_handle);
                            usart1_send_string("ready to recv data\r\n");
                            state = WAIT_FIRST_BYTE;
                            start_idx = 0;
                        }
                    } else {
                        start_idx = 0;
                        if (rx_byte == start_pattern[0]) start_idx = 1;
                    }
                }
                break;

            case WAIT_FIRST_BYTE:
                /* 等待第一个数据字节，超时10秒，允许用户准备发送文件 */
                if (xQueueReceive(usart1_queue_handle, &rx_byte, pdMS_TO_TICKS(10000)) == pdTRUE)
                {
                    /* 收到第一个字节，准备接收数据流 */
                    write_addr = 0;
                    total_written = 0;
                    buf_len = 0;
                    end_pos = 0;
                    memset(end_window, 0, END_LEN);
                    state = RECEIVING;
                    /* 将这个字节放入缓冲区或检测结束符 */
                    if (rx_byte == end_pattern[0]) {
                        end_window[end_pos++] = rx_byte;
                    } else {
                        buffer[buf_len++] = rx_byte;
                    }
                }
                else
                {
                    /* 超时未收到数据，放弃本次传输 */
                    usart1_send_string("Timeout waiting for data, abort\r\n");
                    state = WAIT_START;
                }
                break;

            case RECEIVING:
                /* 累积数据，直到满256字节或检测到结束符 */
                {
                    TickType_t timeout = pdMS_TO_TICKS(5000); // 5秒超时，足够手动发送结束符
                    while (buf_len < 256)
                    {
                        if (xQueueReceive(usart1_queue_handle, &rx_byte, timeout) != pdTRUE)
                        {
                            /* 超时，认为传输结束（可能用户未发送结束符） */
                            if (buf_len > 0) {
                                spiflash_write(buffer, write_addr, buf_len);
                                total_written += buf_len;
                                usart1_send_string("Timeout, final write done\r\n");
                            }
                            usart1_send_string("Timeout, programming aborted\r\n");
                            state = WAIT_START;
                            goto next_cycle;
                        }

                        /* 滑动窗口检测结束符 */
                        if (end_pos < END_LEN) {
                            end_window[end_pos++] = rx_byte;
                        } else {
                            for (int i = 0; i < END_LEN-1; i++) end_window[i] = end_window[i+1];
                            end_window[END_LEN-1] = rx_byte;
                        }

                        if (end_pos >= END_LEN && memcmp(end_window, end_pattern, END_LEN) == 0)
                        {
                            /* 检测到结束符，写入剩余数据 */
                            if (buf_len > 0) {
                                spiflash_write(buffer, write_addr, buf_len);
                                total_written += buf_len;
                                usart1_send_string("Final write done\r\n");
                            }
                            usart1_send_string("flash programming finished, total bytes: ");
                            usart1_send_uint32(total_written);
                            usart1_send_string("\r\n");
                            state = WAIT_START;
                            goto next_cycle;
                        }

                        /* 普通数据，存入缓冲区 */
                        buffer[buf_len++] = rx_byte;
                        if (buf_len == 256) {
                            /* 满256字节，写入Flash */
                            spiflash_write(buffer, write_addr, 256);
                            write_addr += 256;
                            total_written += 256;
                            usart1_send_string("256 bytes written to 0x");
                            usart1_send_hex32(write_addr - 256);
                            usart1_send_string("\r\n");
                            buf_len = 0;  // 清空缓冲区，继续下一块
                        }
                    }
                }
                next_cycle:
                vTaskDelay(1);
                break;
        }
        vTaskDelay(1);
    }

  /* add user code end flash_function 1 */
}



/* add user code begin 2 */
void breath_led(gpio_type *gpio_x, uint16_t pin)
{
    static uint32_t counter = 0;
    static uint16_t brightness = 0;
    static int8_t direction = 1;
    static uint8_t pwm_counter = 0;
    static uint8_t pwm_cycle = 20;
    static uint16_t compare_value = 0;
    pwm_counter++;

    if (pwm_counter >= pwm_cycle)
    {
        pwm_counter = 0;
    }

    if (pwm_counter * 50 < compare_value)
    {
        gpio_bits_set(gpio_x, pin);
    }
    else
    {
        gpio_bits_reset(gpio_x, pin);
    }

    counter++;
    uint8_t safe_speed = 5;

    if (safe_speed > 10)
    {
        safe_speed = 10;
    }

    if (safe_speed < 1)
    {
        safe_speed = 1;
    }

    uint16_t update_interval = 20 / safe_speed;

    if (update_interval < 1)
    {
        update_interval = 1;
    }

    if (counter % update_interval == 0)
    {
        uint16_t step = safe_speed;
        brightness += direction * step;

        if (direction > 0)
        {
            if (brightness >= 1000)
            {
                brightness = 1000;
                direction = -1;
            }
        }
        else
        {
            if (brightness <= 0)
            {
                brightness = 0;
                direction = 1;
            }
        }

        if (brightness < 200)
        {
            compare_value = brightness / 4;
        }
        else if (brightness < 600)
        {
            compare_value = 50 + (brightness - 200) / 2;
        }
        else
        {
            compare_value = 250 + (brightness - 600) * 750 / 400;
        }

        if (counter > 60000)
        {
            counter = 0;
        }
    }
}



static void usart1_send_char(char ch)
{
    usart_data_transmit(USART1, (uint16_t)ch);
    while (usart_flag_get(USART1, USART_TDC_FLAG) == RESET);
}

/**
 * @brief 发送字符串（阻塞方式）
 * @param str 以 '\0' 结尾的字符串
 */
void usart1_send_string(const char *str)
{
    while (*str) {
        usart1_send_char(*str++);
    }
}

/**
 * @brief 发送无符号 32 位整数（十进制格式）
 * @param num 要发送的数字
 */
void usart1_send_uint32(uint32_t num)
{
    char buf[12];   // 最大 10 位数字 + 结束符
    char *p = buf + sizeof(buf) - 1;
    *p = '\0';

    if (num == 0) {
        *(--p) = '0';
    } else {
        while (num > 0) {
            *(--p) = '0' + (num % 10);
            num /= 10;
        }
    }
    usart1_send_string(p);
}

/**
 * @brief 发送无符号 32 位整数（十六进制格式，带 "0x" 前缀）
 * @param num 要发送的数字
 */
void usart1_send_hex32(uint32_t num)
{
    usart1_send_string("0x");
    char hex[9];   // 8 个十六进制字符 + 结束符
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (num >> (i * 4)) & 0x0F;
        hex[7 - i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
    }
    hex[8] = '\0';
    usart1_send_string(hex);
}

/**
 * @brief 以十六进制格式发送缓冲区数据（每行16字节）
 * @param buf 数据缓冲区
 * @param len 数据长度
 */
static void usart1_send_hex_buffer(uint8_t *buf, uint32_t len)
{
    char hex_str[5];  // 用于存放 "XX "
    for (uint32_t i = 0; i < len; i++) {
        if (i % 16 == 0) {
            usart1_send_string("\r\n");  // 换行
        }
        uint8_t high = (buf[i] >> 4) & 0x0F;
        uint8_t low  = buf[i] & 0x0F;
        hex_str[0] = (high < 10) ? ('0' + high) : ('A' + high - 10);
        hex_str[1] = (low  < 10) ? ('0' + low)  : ('A' + low  - 10);
        hex_str[2] = ' ';
        hex_str[3] = '\0';
        usart1_send_string(hex_str);
    }
    usart1_send_string("\r\n");
}
/* add user code end 2 */

