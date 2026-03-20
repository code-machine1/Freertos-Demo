/**
 * @file wifi_handle.c
 * @brief WiFi module high-level application (manual HTTP provisioning, enhanced debug)
 */

#include "wifi_handle.h"
#include "wifi_uart.h"
#include "freertos_app.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

/* Debug switch */
#define WIFI_HANDLE_DEBUG   1
#if WIFI_HANDLE_DEBUG
#include <stdio.h>
#define HANDLE_PRINT(fmt, ...) printf("[WiFi] " fmt "\r\n", ##__VA_ARGS__)
#else
#define HANDLE_PRINT(fmt, ...)
#endif

/* Provisioning parameters */
#define PROV_AP_IP                "192.168.4.1"
#define PROV_AP_GATEWAY           "192.168.4.1"
#define PROV_AP_NETMASK           "255.255.255.0"
#define PROV_DHCP_START           100
#define PROV_DHCP_END             200
#define PROV_HTTP_PORT            80
#define PROV_MAX_APS               20
#define PROV_RESP_BUFFER_SIZE      2048
#define WIFI_RESP_BUFFER_SIZE      256
#define WIFI_AT_TIMEOUT            1000
#define TCP_SERVER_RETRY           5      /* Number of retries for creating TCP server */
#define SCAN_RETRY                 2      /* Number of scan retries */

extern QueueHandle_t uart4_queue_handle;

/* State variables */
static provision_state_t prov_state = PROV_STATE_IDLE;
static provision_result_t prov_result;
static TimerHandle_t prov_timer;
static uint32_t prov_timeout_seconds;
static wifi_ap_info_t scanned_aps[PROV_MAX_APS];
static int scanned_ap_count = 0;
static uint8_t http_server_ready = 0;
static char prov_ap_ssid[33];
static char prov_ap_password[65];
static uint16_t server_conid = 0;

/* Static function declarations */
static int prov_send_at_cmd(const char *cmd, uint32_t timeout_ms);
static int prov_set_wifi_mode(uint8_t mode);
static int prov_config_ap(const char *ssid, const char *password, uint8_t channel);
static int prov_enable_dhcp_server(void);
static int prov_create_tcp_server(uint16_t port);
static int prov_set_socket_recv_mode(void);
static int prov_send_socket_response(uint16_t conid, const char *data, uint16_t len);
static int prov_scan_networks(void);
static int prov_connect_to_ap(const char *ssid, const char *password);
static void prov_timer_callback(TimerHandle_t xTimer);
static void prov_handle_http_request(const char *request, char *response, uint16_t *resp_len);
static void prov_generate_ap_list_html(char *buffer, uint16_t buf_size);
static int prov_parse_post_data(const char *post_data, char *ssid, uint32_t ssid_len,
                                char *password, uint32_t pwd_len);
static void prov_generate_config_page(char *buffer, uint16_t buf_size);
static void prov_generate_success_page(char *buffer, uint16_t buf_size, const char *ip);
static int prov_setup_ap(const char *ssid, const char *password);

/*------------------ Basic AT command wrapper ------------------*/
static int prov_send_at_cmd(const char *cmd, uint32_t timeout_ms)
{
    char resp[64];
    int ret = wifi_send_at_cmd(cmd, resp, sizeof(resp), timeout_ms);
    HANDLE_PRINT("CMD: %s, ret=%d, resp: %s", cmd, ret, resp);
    return ret;
}

static int prov_set_wifi_mode(uint8_t mode)
{
    char cmd[16];
    int ret;
    snprintf(cmd, sizeof(cmd), "AT+WMODE=%d", mode);
    ret = prov_send_at_cmd(cmd, 2000);

    if (ret == 0)
    {
        return 0;
    }

    HANDLE_PRINT("Direct mode set failed, try turning WiFi off first");

    if (prov_send_at_cmd("AT+WMODE=0", 2000) != 0)
    {
        HANDLE_PRINT("Failed to turn off WiFi");
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(500));
    snprintf(cmd, sizeof(cmd), "AT+WMODE=%d", mode);
    return prov_send_at_cmd(cmd, 2000);
}

static int prov_config_ap(const char *ssid, const char *password, uint8_t channel)
{
    char cmd[128];

    if (password == NULL || strlen(password) == 0)
    {
        snprintf(cmd, sizeof(cmd), "AT+WAP=\"%s\",\"\",%d,4,0", ssid, channel);
    }
    else
    {
        snprintf(cmd, sizeof(cmd), "AT+WAP=\"%s\",\"%s\",%d,4,0", ssid, password, channel);
    }

    return prov_send_at_cmd(cmd, 3000);
}

static int prov_enable_dhcp_server(void)
{
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "AT+WAPDHCP=1,192.168.4.%d,192.168.4.%d,%s",
             PROV_DHCP_START, PROV_DHCP_END, PROV_AP_IP);
    return prov_send_at_cmd(cmd, 2000);
}

static int prov_create_tcp_server(uint16_t port)
{
    char cmd[32];
    char resp[64];
    snprintf(cmd, sizeof(cmd), "AT+SOCKET=3,%d", port);
    int ret = wifi_send_at_cmd(cmd, resp, sizeof(resp), 3000);

    if (ret == 0)
    {
        char *p = strstr(resp, "ConID=");

        if (p)
        {
            p += 6;
            server_conid = atoi(p);
            HANDLE_PRINT("TCP Server created, ConID=%d", server_conid);
        }
        else
        {
            HANDLE_PRINT("TCP Server created but ConID not found in response: %s", resp);
        }

        return 0;
    }
    else
    {
        HANDLE_PRINT("TCP Server creation failed, ret=%d, resp=%s", ret, resp);
        return -1;
    }
}

static int prov_set_socket_recv_mode(void)
{
    return prov_send_at_cmd("AT+SOCKETRECVCFG=0", 1000);
}

static int prov_send_socket_response(uint16_t conid, const char *data, uint16_t len)
{
    char cmd[32];
    snprintf(cmd, sizeof(cmd), "AT+SOCKETSEND=%d,%d", conid, len);
    wifi_send_raw((const uint8_t*)cmd, strlen(cmd));
    wifi_send_raw((const uint8_t*)"\r\n", 2);
    // 等待 '>' 提示符
    TickType_t start = xTaskGetTickCount();
    uint8_t got_prompt = 0;

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(2000))
    {
        uint8_t ch;

        if (xQueueReceive(uart4_queue_handle, &ch, pdMS_TO_TICKS(10)) == pdTRUE)
        {
            if (ch == '>')
            {
                got_prompt = 1;
                break;
            }
        }
    }

    if (!got_prompt)
    {
        HANDLE_PRINT("No '>' prompt");
        return -1;
    }

    wifi_send_raw((const uint8_t*)data, len);
    // 等待模块返回 OK
    char resp[32];
    int ok_received = 0;
    start = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(3000))
    {
        int rlen = wifi_recv_raw((uint8_t*)resp, sizeof(resp) -1, 200);

        if (rlen > 0)
        {
            resp[rlen] = '\0';

            if (strstr(resp, "OK") != NULL)
            {
                ok_received = 1;
                break;
            }
        }
    }

    if (ok_received)
    {
        HANDLE_PRINT("Socket send OK");
        return 0;
    }
    else
    {
        HANDLE_PRINT("Socket send failed (no OK)");
        return -1;
    }
}

static int prov_scan_networks(void)
{
    char resp[2048];
    HANDLE_PRINT("Starting WiFi scan...");
    int ret = wifi_send_at_cmd("AT+WSCAN", resp, sizeof(resp), 8000);

    if (ret != 0)
    {
        HANDLE_PRINT("Scan failed, ret=%d", ret);
        return -1;
    }

    scanned_ap_count = 0;
    char *line = strtok(resp, "\r\n");
    int first_line = 1;

    while (line != NULL && scanned_ap_count < PROV_MAX_APS)
    {
        if (first_line)
        {
            first_line = 0;
            line = strtok(NULL, "\r\n");
            continue;
        }

        int index, ch, rssi;
        char ssid[33] = {0};
        char security[32] = {0};
        char bssid[18] = {0};

        if (sscanf(line, "%d,%[^,],%d,%[^,],%d,%s",
                   &index, ssid, &ch, security, &rssi, bssid) == 6)
        {
            strncpy(scanned_aps[scanned_ap_count].ssid, ssid, 32);
            scanned_aps[scanned_ap_count].rssi = (int8_t)rssi;
            scanned_ap_count++;
        }

        line = strtok(NULL, "\r\n");
    }

    // Sort by signal strength (descending)
    for (int i = 0; i < scanned_ap_count - 1; i++)
    {
        for (int j = i + 1; j < scanned_ap_count; j++)
        {
            if (scanned_aps[i].rssi < scanned_aps[j].rssi)
            {
                wifi_ap_info_t tmp = scanned_aps[i];
                scanned_aps[i] = scanned_aps[j];
                scanned_aps[j] = tmp;
            }
        }
    }

    HANDLE_PRINT("Scan completed, found %d APs", scanned_ap_count);
    return scanned_ap_count;
}

static int prov_connect_to_ap(const char *ssid, const char *password) {
    prov_result.got_ip = 0; // 初始化IP获取标志

    // 先切换到STA模式（不保存到flash）
    HANDLE_PRINT("Switching to STA mode");
    if (prov_send_at_cmd("AT+WMODE=1,0", 2000) != 0) {
        HANDLE_PRINT("Failed to set STA mode");
        return -1;
    }
    vTaskDelay(pdMS_TO_TICKS(500)); // 等待模式切换生效

    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+WJAP=\"%s\",\"%s\"", ssid, password);
    HANDLE_PRINT("Connecting to AP: %s", ssid);
    int ret = prov_send_at_cmd(cmd, 15000); // 等待连接结果
    if (ret != 0) {
        HANDLE_PRINT("WJAP command failed");
        return -1;
    }

    // 等待IP获取，最多10秒
    TickType_t start = xTaskGetTickCount();
    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(10000)) {
        char resp[256];
        if (wifi_send_at_cmd("AT+WJAP?", resp, sizeof(resp), 2000) == 0) {
            int status = -1;
            char ip_addr[16] = {0};
            char *p = strstr(resp, "+WJAP:");
            if (p) {
                p += 6; // 跳过 "+WJAP:"
                // 提取状态
                char *comma = strchr(p, ',');
                if (comma) {
                    *comma = '\0';
                    status = atoi(p);
                    p = comma + 1;
                    // 跳过ssid, pwd, bssid, security, MAC, ch 共6个字段
                    for (int i = 0; i < 6; i++) {
                        comma = strchr(p, ',');
                        if (!comma) break;
                        p = comma + 1;
                    }
                    // 现在 p 指向 IP 字段
                    comma = strchr(p, ',');
                    if (comma) {
                        *comma = '\0';
                        strncpy(ip_addr, p, sizeof(ip_addr)-1);
                    }
                }
            }
            // 如果状态为3且IP不是0.0.0.0
            if (status == 3 && strcmp(ip_addr, "0.0.0.0") != 0 && strchr(ip_addr, '.') != NULL) {
                strncpy(prov_result.ip, ip_addr, sizeof(prov_result.ip)-1);
                prov_result.got_ip = 1;
                HANDLE_PRINT("Got IP: %s", prov_result.ip);
                return 0;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
    HANDLE_PRINT("IP acquisition timeout");
    return -1;
}

static void prov_timer_callback(TimerHandle_t xTimer)
{
    if (prov_state != PROV_STATE_CONNECTED && prov_state != PROV_STATE_IDLE)
    {
        HANDLE_PRINT("Provisioning timeout");
        prov_state = PROV_STATE_TIMEOUT;
    }
}

/*------------------ HTTP page generation ------------------*/

static void prov_generate_ap_list_html(char *buffer, uint16_t buf_size)
{
    int offset = snprintf(buffer, buf_size,
                          "<select name=\"ssid\" id=\"ssid\" style=\"width:100%%; padding:8px; margin:10px 0;\">\r\n"
                          "<option value=\"\">-- Select WiFi Network --</option>\r\n");

    for (int i = 0; i < scanned_ap_count; i++)
    {
        int bars = 0;

        if (scanned_aps[i].rssi >= -50)
        {
            bars = 4;
        }
        else if (scanned_aps[i].rssi >= -60)
        {
            bars = 3;
        }
        else if (scanned_aps[i].rssi >= -70)
        {
            bars = 2;
        }
        else if (scanned_aps[i].rssi >= -80)
        {
            bars = 1;
        }

        char bars_str[16] = {0};

        for (int b = 0; b < bars; b++)
        {
            bars_str[b] = '|';
        }

        offset += snprintf(buffer + offset, buf_size - offset,
                           "<option value=\"%s\">%s [%d dBm] %s</option>\r\n",
                           scanned_aps[i].ssid, scanned_aps[i].ssid, scanned_aps[i].rssi, bars_str);
    }

    snprintf(buffer + offset, buf_size - offset, "</select>\r\n");
}

static void prov_generate_config_page(char *buffer, uint16_t buf_size)
{
    char ap_list_html[512] = {0};
    int offset = 0;

    for (int i = 0; i < scanned_ap_count; i++)
    {
        offset += snprintf(ap_list_html + offset, sizeof(ap_list_html) - offset,
                           "<option value=\"%s\">%s (%d dBm)</option>",
                           scanned_aps[i].ssid, scanned_aps[i].ssid, scanned_aps[i].rssi);
    }

    snprintf(buffer, buf_size,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html\r\n"
             "Connection: close\r\n"
             "\r\n"
             "<!DOCTYPE html><html><body>"
             "<h2>FORWARD G1</h2>"
             "<form id=f><select id=ssid>%s</select><br>"
             "Password:<input id=pwd><br>"
             "<button type=button onclick=\"c()\">Connect</button>"
             "</form><div id=s></div>"
             "<script>"
             "function c(){"
             "var s=document.getElementById('ssid').value;"
             "var p=document.getElementById('pwd').value;"
             "if(!s||!p){alert('Fill both');return;}"
             "var x=new XMLHttpRequest();"
             "x.open('POST','/config',true);"
             "x.setRequestHeader('Content-Type','application/x-www-form-urlencoded');"
             "x.onreadystatechange=function(){if(x.readyState==4) document.getElementById('s').innerHTML=x.responseText;};"
             "x.send('ssid='+encodeURIComponent(s)+'&password='+encodeURIComponent(p));"
             "}"
             "</script>"
             "</body></html>", ap_list_html);
}

static void prov_generate_success_page(char *buffer, uint16_t buf_size, const char *ip)
{
    snprintf(buffer, buf_size,
             "HTTP/1.1 200 OK\r\n"
             "Content-Type: text/html; charset=UTF-8\r\n"
             "\r\n"
             "<!DOCTYPE html>\r\n"
             "<html>\r\n"
             "<head><meta charset=\"UTF-8\"><title>Connection Successful</title>"
             "<meta http-equiv=\"refresh\" content=\"5;url=/\">"
             "<style>body{font-family:Arial;text-align:center;padding:50px;}.success{color:#4CAF50;}</style>"
             "</head>\r\n"
             "<body>\r\n"
             "<h1 class=\"success\">? Connected!</h1>\r\n"
             "<p>Device has connected to the WiFi network</p>\r\n"
             "<p>Obtained IP: %s</p>\r\n"
             "<p>Returning to main page in 5 seconds...</p>\r\n"
             "</body>\r\n"
             "</html>\r\n", ip);
}

static int prov_parse_post_data(const char *post_data, char *ssid, uint32_t ssid_len,
                                char *password, uint32_t pwd_len)
{
    char *p = (char *)post_data;
    char *token;
    // 解析ssid
    token = strstr(p, "ssid=");

    if (token)
    {
        token += 5;
        char *end = strchr(token, '&');

        if (!end)
        {
            end = strchr(token, '\r');
        }

        if (!end)
        {
            end = token + strlen(token);
        }

        int len = end - token;

        if (len >= ssid_len)
        {
            len = ssid_len - 1;
        }

        strncpy(ssid, token, len);
        ssid[len] = '\0';
        // URL解码
        char *src = ssid, *dst = ssid;

        while (*src)
        {
            if (*src == '+')
            {
                *dst++ = ' ';
                src++;
            }
            else if (*src == '%' && *(src + 1) && *(src + 2))
            {
                char hex[3] = {src[1], src[2], '\0'};
                *dst++ = (char)strtol(hex, NULL, 16);
                src += 3;
            }
            else
            {
                *dst++ = *src++;
            }
        }

        *dst = '\0';
    }

    // 解析password（精确到下一个 \r 或 \n 前，保留所有字符）
    token = strstr(p, "password=");

    if (token)
    {
        token += 9; // 指向密码开始
        char *end = strchr(token, '\r');

        if (!end)
        {
            end = strchr(token, '\n');
        }

        if (!end)
        {
            end = token + strlen(token);
        }

        int len = end - token;

        if (len >= pwd_len)
        {
            len = pwd_len - 1;
        }

        strncpy(password, token, len);
        password[len] = '\0';
        // URL解码（处理特殊字符如 %20 等）
        char *src = password, *dst = password;

        while (*src)
        {
            if (*src == '+')
            {
                *dst++ = ' ';
                src++;
            }
            else if (*src == '%' && *(src + 1) && *(src + 2))
            {
                char hex[3] = {src[1], src[2], '\0'};
                *dst++ = (char)strtol(hex, NULL, 16);
                src += 3;
            }
            else
            {
                *dst++ = *src++;
            }
        }

        *dst = '\0';
    }

    return (strlen(ssid) > 0) ? 0 : -1;
}

static void prov_handle_http_request(const char *request, char *response, uint16_t *resp_len)
{
    HANDLE_PRINT("Received request: %s", request);

    if (strncmp(request, "GET / ", 6) == 0 || strstr(request, "GET / HTTP") != NULL)
    {
        prov_generate_config_page(response, PROV_RESP_BUFFER_SIZE);
        *resp_len = strlen(response);
    }
    else if (strncmp(request, "POST /config", 12) == 0)
    {
        char *body = strstr(request, "\r\n\r\n");

        if (body)
        {
            body += 4;
            char ssid[33] = {0};
            char password[65] = {0};

            if (prov_parse_post_data(body, ssid, sizeof(ssid), password, sizeof(password)) == 0)
            {
                HANDLE_PRINT("Received config: SSID=%s, password length=%d", ssid, strlen(password));
                strncpy(prov_result.ssid, ssid, sizeof(prov_result.ssid) - 1);
                strncpy(prov_result.password, password, sizeof(prov_result.password) - 1);
                snprintf(response, PROV_RESP_BUFFER_SIZE,
                         "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "success");
                *resp_len = strlen(response);
                prov_state = PROV_STATE_CONNECTING;   // trigger connection
            }
            else
            {
                snprintf(response, PROV_RESP_BUFFER_SIZE,
                         "HTTP/1.1 400 Bad Request\r\n"
                         "Content-Type: text/plain\r\n"
                         "\r\n"
                         "error: missing parameters");
                *resp_len = strlen(response);
            }
        }
    }
    else
    {
        snprintf(response, PROV_RESP_BUFFER_SIZE,
                 "HTTP/1.1 404 Not Found\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n"
                 "404 Not Found");
        *resp_len = strlen(response);
    }
}

/*------------------ Basic functions ------------------*/

int wifi_handle_init(void)
{
    if (wifi_uart_init() != 0)
    {
        HANDLE_PRINT("UART init failed");
        return -1;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));

    for (int i = 0; i < 3; i++)
    {
        wifi_send_raw((const uint8_t*)"\r\n", 2);
        vTaskDelay(pdMS_TO_TICKS(200));
        wifi_uart_clear_rx();
    }

    int retry = 10;
    char response[WIFI_RESP_BUFFER_SIZE];

    for (int i = 0; i < retry; i++)
    {
        int ret = wifi_send_at_cmd("AT", response, sizeof(response), 1000);

        if (ret == 0)
        {
            HANDLE_PRINT("Module ready, response: %s", response);
            return 0;
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }

    HANDLE_PRINT("Module init failed");
    return -1;
}

int wifi_test_at(void)
{
    char response[WIFI_RESP_BUFFER_SIZE] = {0};
    return wifi_send_at_cmd("AT", response, sizeof(response), WIFI_AT_TIMEOUT);
}

int wifi_get_version(char *version, uint32_t len)
{
    if (version == NULL || len == 0)
    {
        return -1;
    }

    return wifi_send_at_cmd("AT+GMR", version, len, WIFI_AT_TIMEOUT);
}

int wifi_set_mode(uint8_t mode)
{
    if (mode < 1 || mode > 3)
    {
        return -1;
    }

    char cmd[16];
    snprintf(cmd, sizeof(cmd), "AT+WMODE=%d", mode);
    return wifi_send_at_cmd(cmd, NULL, 0, WIFI_AT_TIMEOUT);
}

int wifi_connect_ap(const char *ssid, const char *password, uint32_t timeout_ms)
{
    char cmd[128];
    snprintf(cmd, sizeof(cmd), "AT+WJAP=\"%s\",\"%s\"", ssid, password);
    return wifi_send_at_cmd(cmd, NULL, 0, timeout_ms);
}

int wifi_get_ip(char *ip, uint32_t len)
{
    if (ip == NULL || len == 0)
    {
        return -1;
    }

    char response[WIFI_RESP_BUFFER_SIZE];
    int ret = wifi_send_at_cmd("AT+CIFSR", response, sizeof(response), WIFI_AT_TIMEOUT);

    if (ret != 0)
    {
        return ret;
    }

    const char *p = response;

    while (*p)
    {
        if (*p >= '0' && *p <= '9')
        {
            const char *start = p;

            while (*p && ((*p >= '0' && *p <= '9') || *p == '.'))
            {
                p++;
            }

            const char *dot = strchr(start, '.');

            if (dot && dot < p)
            {
                uint32_t l = p - start;

                if (l < len)
                {
                    strncpy(ip, start, l);
                    ip[l] = '\0';
                    return 0;
                }
            }
        }
        else
        {
            p++;
        }
    }

    return 1;
}

void wifi_test_task_entry(void *pvParameters)
{
    if (wifi_handle_init() != 0)
    {
        vTaskDelete(NULL);
    }

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

/*------------------ Provisioning interface ------------------*/

int wifi_provision_start(const char *ap_ssid, const char *ap_password, uint32_t timeout_s)
{
    if (prov_state != PROV_STATE_IDLE)
    {
        HANDLE_PRINT("Provisioning already in progress");
        return -1;
    }

    strncpy(prov_ap_ssid, ap_ssid, sizeof(prov_ap_ssid) - 1);

    if (ap_password)
    {
        strncpy(prov_ap_password, ap_password, sizeof(prov_ap_password) - 1);
    }
    else
    {
        prov_ap_password[0] = '\0';
    }

    memset(&prov_result, 0, sizeof(prov_result));
    scanned_ap_count = 0;
    prov_timeout_seconds = timeout_s;
    http_server_ready = 0;
    server_conid = 0;

    if (timeout_s > 0)
    {
        prov_timer = xTimerCreate("ProvTimer", pdMS_TO_TICKS(timeout_s * 1000),
                                  pdFALSE, NULL, prov_timer_callback);

        if (prov_timer)
        {
            xTimerStart(prov_timer, 0);
        }
    }

    prov_state = PROV_STATE_AP_MODE;
    HANDLE_PRINT("Provisioning started, AP SSID: %s", ap_ssid);
    return 0;
}

void wifi_provision_stop(void)
{
    if (prov_timer)
    {
        xTimerStop(prov_timer, 0);
        xTimerDelete(prov_timer, 0);
    }

    prov_send_at_cmd("AT+RST", 3000);  // reset to exit AP mode
    prov_state = PROV_STATE_IDLE;
    HANDLE_PRINT("Provisioning stopped");
}

provision_state_t wifi_provision_get_state(void)
{
    return prov_state;
}

int wifi_provision_get_result(provision_result_t *result)
{
    if (prov_state != PROV_STATE_CONNECTED || result == NULL)
    {
        return -1;
    }

    *result = prov_result;
    return 0;
}

void wifi_provision_process(void)
{
    static char recv_buf[512];
    static uint8_t need_scan = 1;
    static uint8_t tcp_server_retry = 0;

    switch (prov_state)
    {
    case PROV_STATE_AP_MODE:
    {
        HANDLE_PRINT("Entering AP_MODE state");

        if (prov_setup_ap(prov_ap_ssid, prov_ap_password[0] ? prov_ap_password : NULL) == 0)
        {
            prov_state = PROV_STATE_HTTP_SERVER;
        }
        else
        {
            HANDLE_PRINT("AP setup failed");
            prov_state = PROV_STATE_CONNECT_FAILED;
        }

        break;
    }

    case PROV_STATE_HTTP_SERVER:
    {
        if (!http_server_ready)
        {
            if (need_scan)
            {
                int scan_ok = 0;

                for (int i = 0; i < SCAN_RETRY; i++)
                {
                    if (prov_scan_networks() > 0)
                    {
                        scan_ok = 1;
                        break;
                    }

                    vTaskDelay(pdMS_TO_TICKS(500));
                }

                if (!scan_ok)
                {
                    HANDLE_PRINT("Scan failed after %d retries, aborting", SCAN_RETRY);
                    prov_state = PROV_STATE_CONNECT_FAILED;
                    break;
                }

                need_scan = 0;
            }

            int ret = -1;

            for (int i = 0; i < TCP_SERVER_RETRY; i++)
            {
                HANDLE_PRINT("Creating TCP server, attempt %d/%d", i + 1, TCP_SERVER_RETRY);
                ret = prov_create_tcp_server(PROV_HTTP_PORT);

                if (ret == 0)
                {
                    break;
                }

                vTaskDelay(pdMS_TO_TICKS(500));
            }

            if (ret == 0)
            {
                /* 设置为被动模式 */
                if (prov_set_socket_recv_mode() == 0)
                {
                    http_server_ready = 1;
                    HANDLE_PRINT("HTTP server is ready! Connect to AP and visit http://%s", PROV_AP_IP);
                }
                else
                {
                    HANDLE_PRINT("Failed to set socket receive mode");
                    prov_state = PROV_STATE_CONNECT_FAILED;
                }
            }
            else
            {
                HANDLE_PRINT("TCP Server creation failed after %d retries", TCP_SERVER_RETRY);
                prov_state = PROV_STATE_CONNECT_FAILED;
            }
        }
        else
        {
            int len = wifi_recv_raw((uint8_t*)recv_buf, sizeof(recv_buf) -1, 50);

            if (len > 0)
            {
                recv_buf[len] = '\0';
                char *p = recv_buf;

                while ((p = strstr(p, "+EVENT:SocketDown,")) != NULL)
                {
                    int conid;

                    if (sscanf(p, "+EVENT:SocketDown,%d", &conid) == 1)
                    {
                        HANDLE_PRINT("SocketDown event, conid=%d", conid);
                        // 发送 AT+SOCKETREAD 命令
                        // 发送 AT+SOCKETREAD 命令
                        char read_cmd[32];
                        snprintf(read_cmd, sizeof(read_cmd), "AT+SOCKETREAD=%d", conid);
                        char read_resp[2048];
                        int ret = wifi_send_at_cmd(read_cmd, read_resp, sizeof(read_resp), 1000);

                        if (ret == 0)
                        {
                            // 解析响应：+SOCKETREAD:<conID>,<len>,<data>
                            char *p = read_resp;
                            // 跳过 "+SOCKETREAD:"
                            p = strchr(p, ',');

                            if (!p)
                            {
                                continue;
                            }

                            p++; // 跳过第一个逗号
                            p = strchr(p, ',');

                            if (!p)
                            {
                                continue;
                            }

                            p++; // 跳过第二个逗号
                            p = strchr(p, ',');

                            if (!p)
                            {
                                continue;
                            }

                            p++; // 跳过第三个逗号，现在指向数据开始

                            // 去除可能的前导空白
                            while (*p == ' ' || *p == '\r' || *p == '\n')
                            {
                                p++;
                            }

                            // 复制到 request 缓冲区
                            char request[1024];
                            strncpy(request, p, sizeof(request) -1);
                            request[sizeof(request) -1] = '\0';
                            HANDLE_PRINT("Received request: %s", request);  // 应打印纯请求
                            char response[PROV_RESP_BUFFER_SIZE];
                            uint16_t resp_len = 0;
                            prov_handle_http_request(request, response, &resp_len);

                            if (resp_len > 0)
                            {
                                prov_send_socket_response(conid, response, resp_len);
                            }
                        }
                        else
                        {
                            HANDLE_PRINT("SOCKETREAD failed, ret=%d", ret);
                        }
                    }

                    p = strchr(p, '\n');

                    if (p)
                    {
                        p++;
                    }
                    else
                    {
                        break;
                    }
                }
            }
        }

        break;
    }

    case PROV_STATE_CONNECTING:
    {
        HANDLE_PRINT("Connecting to target AP: %s", prov_result.ssid);

        if (prov_connect_to_ap(prov_result.ssid, prov_result.password) == 0)
        {
            prov_state = PROV_STATE_CONNECTED;
            HANDLE_PRINT("Connected successfully, IP: %s", prov_result.ip);

            if (prov_timer)
            {
                xTimerStop(prov_timer, 0);
            }

            HANDLE_PRINT("Provisioning successful, you may disconnect from AP");
        }
        else
        {
            prov_state = PROV_STATE_CONNECT_FAILED;
            HANDLE_PRINT("Connection to AP failed");
        }

        http_server_ready = 0;
        need_scan = 1;
        break;
    }

    case PROV_STATE_CONNECT_FAILED:
        HANDLE_PRINT("Connect failed, staying in provisioning mode. Please try again.");
        prov_state = PROV_STATE_HTTP_SERVER;
        http_server_ready = 1;  // 保持HTTP服务器运行，不重新创建
        // need_scan 保持不变，保留当前扫描列表
        break;

    case PROV_STATE_TIMEOUT:
        HANDLE_PRINT("Provisioning timeout");
        wifi_provision_stop();
        break;

    default:
        break;
    }
}

/* Helper: setup AP mode (must be declared before use) */
static int prov_setup_ap(const char *ssid, const char *password)
{
    HANDLE_PRINT("Setting up AP mode, SSID=%s", ssid);

    // 复位模块
    if (prov_send_at_cmd("AT+RST", 5000) != 0)
    {
        HANDLE_PRINT("Reset failed");
        return -1;
    }

    // 等待ready
    char boot_buf[256];
    int ready_found = 0;
    TickType_t start = xTaskGetTickCount();

    while ((xTaskGetTickCount() - start) < pdMS_TO_TICKS(15000))
    {
        int len = wifi_recv_raw((uint8_t*)boot_buf, sizeof(boot_buf) -1, 200);

        if (len > 0)
        {
            boot_buf[len] = '\0';
            HANDLE_PRINT("Boot message: %s", boot_buf);

            if (strstr(boot_buf, "ready") != NULL)
            {
                ready_found = 1;
                break;
            }
        }
    }

    if (!ready_found)
    {
        HANDLE_PRINT("ready not detected");
        return -1;
    }

    wifi_uart_clear_rx();
    vTaskDelay(pdMS_TO_TICKS(500));
    // 禁用自动连接
    prov_send_at_cmd("AT+WAUTOCONN=0", 1000);
    // 增加延时，确保WiFi初始化
    vTaskDelay(pdMS_TO_TICKS(3000));

    // 设置WiFi模式为AP（不保存到flash）- 修正为两个参数
    if (prov_send_at_cmd("AT+WMODE=2,0", 2000) != 0)
    {
        HANDLE_PRINT("Failed to set AP mode");
        return -1;
    }

    // 配置 AP
    if (prov_config_ap(ssid, password, 6) != 0)
    {
        HANDLE_PRINT("AP config failed");
        return -1;
    }

    // 启用 DHCP
    if (prov_enable_dhcp_server() != 0)
    {
        HANDLE_PRINT("DHCP enable failed");
        return -1;
    }

    HANDLE_PRINT("AP started: SSID=%s, IP=%s", ssid, PROV_AP_IP);
    vTaskDelay(pdMS_TO_TICKS(2000));
    // 查询 AP 状态
    char ap_status[256];

    if (wifi_send_at_cmd("AT+WAP?", ap_status, sizeof(ap_status), 2000) == 0)
    {
        HANDLE_PRINT("AP status: %s", ap_status);
    }

    return 0;
}