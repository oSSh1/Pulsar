#pragma once

#include <stdint.h>

#define DEVICE_ID   0x57
#define PART_ID 0x15
#define I2C_PORT    i2c1
#define PIN_SDA     18
#define PIN_SCL     19
#define PIN_INT     15
#define REG_INT_ST1     0x00
#define REG_INT_EN1     0x02
#define REG_FIFO_WR     0x04
#define REG_OVF_CNT     0x05
#define REG_FIFO_RD     0x06
#define REG_FIFO_DATA   0x07
#define REG_FIFO_CFG    0x08
#define REG_MODE_CFG    0x09
#define REG_SPO2_CFG    0x0A
#define REG_LED1_PA     0x0C
#define REG_PART_ID     0xFF
#define BYTES_PER_SAMPLE 3
#define I2C_FREQ (400 * 1000)
#define I2C_BYTE_TIMEOUT 50
#define I2C_SWITCH_COUNT 10

typedef enum {
    MAX30102_OK = 0,
    MAX30102_IS_NOT_CONNECTED,
    WRITE_DATA_LOST,
    WRONG_PART_ID,
    REG_RESET_FAILURE,
    MAX30102_READ_WRITE_TIMEOUT,
    MAX30102_UNKNOWN_ERROR,
    MAX30102_FIFO_NO_DATA,
    MAX30102_RECOVERY_ERROR,
    MAX30102_FIFO_DATA_LOST
} max30102_status;

max30102_status max30102_init(void);
max30102_status max30102_read(uint32_t *out);
max30102_status max30102_i2c_bus_recovery(void);
max30102_status reg_read(uint8_t reg, uint8_t *dst, uint8_t len);

