#include "flash_programmer.h"
#include "at32_spiflash.h"
#include <string.h>

typedef enum {
    WAIT_START,
    RECEIVING
} prog_state_t;

static prog_state_t state = WAIT_START;
static uint32_t write_addr = FLASH_BIN_START_ADDR;
static uint32_t total_written = 0;
static uint8_t buffer[256];
static uint32_t buf_len = 0;

static const uint8_t start_pattern[] = {'f','l','a','s','h',' ','s','t','a','r','t'};
#define START_LEN 11
static uint8_t start_match = 0;

static const uint8_t end_pattern[] = {'f','l','a','s','h',' ','e','n','d'};
#define END_LEN 9
static uint8_t end_window[END_LEN] = {0};
static uint8_t end_pos = 0;

static flash_prog_write_callback_t cb_on_start = NULL;
static flash_prog_write_callback_t cb_on_done = NULL;

static void flush_buffer(void)
{
    if (buf_len > 0) {
        if (cb_on_start) cb_on_start(write_addr, buf_len);
        spiflash_write(buffer, write_addr, buf_len);
        if (cb_on_done)  cb_on_done(write_addr, buf_len);
        write_addr += buf_len;
        total_written += buf_len;
        buf_len = 0;
    }
}

static void buffer_byte(uint8_t byte)
{
    buffer[buf_len++] = byte;
    if (buf_len >= 256) {
        flush_buffer();
    }
}

void flash_programmer_init(void)
{
    state = WAIT_START;
    write_addr = FLASH_BIN_START_ADDR;
    total_written = 0;
    buf_len = 0;
    start_match = 0;
    end_pos = 0;
    memset(end_window, 0, END_LEN);
}

void flash_programmer_set_callbacks(flash_prog_write_callback_t on_start, flash_prog_write_callback_t on_done)
{
    cb_on_start = on_start;
    cb_on_done = on_done;
}

uint8_t flash_programmer_put_byte(uint8_t byte)
{
    uint8_t result = 0;

    if (state == WAIT_START) {
        if (byte == start_pattern[start_match]) {
            start_match++;
            if (start_match == START_LEN) {
                state = RECEIVING;
                write_addr = FLASH_BIN_START_ADDR;
                total_written = 0;
                buf_len = 0;
                end_pos = 0;
                memset(end_window, 0, END_LEN);
                start_match = 0;
                result = 1;
            }
        } else {
            start_match = 0;
            if (byte == start_pattern[0]) start_match = 1;
        }
    }
    else if (state == RECEIVING) {
        if (end_pos < END_LEN) {
            end_window[end_pos++] = byte;
        } else {
            for (int i = 0; i < END_LEN-1; i++) end_window[i] = end_window[i+1];
            end_window[END_LEN-1] = byte;
        }

        if (end_pos >= END_LEN && memcmp(end_window, end_pattern, END_LEN) == 0) {
            flush_buffer();
            state = WAIT_START;
            result = 2;
            return result;
        }

        buffer_byte(byte);
    }

    return result;
}

uint32_t flash_programmer_get_written(void)
{
    return total_written;
}