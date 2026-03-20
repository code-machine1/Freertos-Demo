/**
 * @file wifi_handle.h
 * @brief WiFi module high-level application interface (with manual HTTP provisioning)
 */

#ifndef WIFI_HANDLE_H
#define WIFI_HANDLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Provisioning states */
typedef enum {
    PROV_STATE_IDLE = 0,           // Idle
    PROV_STATE_AP_MODE,             // Starting AP mode
    PROV_STATE_HTTP_SERVER,         // HTTP server running
    PROV_STATE_CONNECTING,          // Connecting to target AP
    PROV_STATE_CONNECTED,           // Connected successfully
    PROV_STATE_CONNECT_FAILED,      // Connection failed
    PROV_STATE_TIMEOUT              // Provisioning timeout
} provision_state_t;

/* WiFi AP info (scan result) */
typedef struct {
    char ssid[33];                  // SSID (max 32 chars + terminator)
    int8_t rssi;                    // Signal strength in dBm
} wifi_ap_info_t;

/* Provisioning result */
typedef struct {
    char ssid[33];
    char password[65];
    uint8_t got_ip;                  // IP acquired flag
    char ip[16];                     // Obtained IP address
} provision_result_t;

/* Basic functions */
int wifi_handle_init(void);
int wifi_test_at(void);
int wifi_get_version(char *version, uint32_t len);
int wifi_set_mode(uint8_t mode);
int wifi_connect_ap(const char *ssid, const char *password, uint32_t timeout_ms);
int wifi_get_ip(char *ip, uint32_t len);
void wifi_test_task_entry(void *pvParameters);

/* Provisioning functions */
int wifi_provision_start(const char *ap_ssid, const char *ap_password, uint32_t timeout_s);
void wifi_provision_stop(void);
provision_state_t wifi_provision_get_state(void);
int wifi_provision_get_result(provision_result_t *result);
void wifi_provision_process(void);

#ifdef __cplusplus
}
#endif

#endif /* WIFI_HANDLE_H */