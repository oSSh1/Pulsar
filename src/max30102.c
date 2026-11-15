#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "max30102.h"

#define DEVICE_ID   0x57
#define PART_ID 0x15
#define I2C_PORT    i2c1
#define PIN_SDA     18
#define PIN_SCL     19
#define PIN_INT     15
#define REG_INT_ST1     0x00
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

static void max30102_i2c_init(void){
    i2c_init(I2C_PORT, I2C_FREQ);
    gpio_set_function(PIN_SDA, GPIO_FUNC_I2C);
    gpio_set_function(PIN_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(PIN_SDA);
    gpio_pull_up(PIN_SCL);
}


max30102_status max30102_i2c_bus_recovery(void){
    gpio_init(PIN_SDA);
    gpio_init(PIN_SCL);

    for (uint8_t i = 0; i < I2C_SWITCH_COUNT; i++){
        gpio_set_dir(PIN_SCL, GPIO_OUT);
        sleep_ms(5);
        gpio_set_dir(PIN_SCL, GPIO_IN);
        sleep_ms(5);
    }
    gpio_set_dir(PIN_SCL, GPIO_OUT);
    sleep_ms(5);
    if (gpio_get(PIN_SDA) == 1){
        gpio_set_dir(PIN_SDA, GPIO_OUT);
        sleep_ms(5);
        gpio_set_dir(PIN_SCL, GPIO_IN);
        sleep_ms(5);
        gpio_set_dir(PIN_SDA, GPIO_IN);
    }else{
        max30102_i2c_init();
        return MAX30102_RECOVERY_ERROR;
    }

    max30102_i2c_init();

    return MAX30102_OK;
}


static max30102_status reg_write_read_check (int code, int len){
    switch (code){
        case (PICO_ERROR_GENERIC):
            return MAX30102_IS_NOT_CONNECTED;
        case (PICO_ERROR_TIMEOUT):
            return MAX30102_READ_WRITE_TIMEOUT;
        default:
            if (code == len){
                return MAX30102_OK;
            }else{
                if (code >= 0 && code < len){
                    return WRITE_DATA_LOST;
                }
            }
            return MAX30102_UNKNOWN_ERROR;
    }
}


static max30102_status reg_write(uint8_t reg, uint8_t val){
    uint8_t wr[2] = {reg, val};
    
    int wr_size = (int)sizeof(wr);
    uint timeout = (uint)((wr_size + 1) * I2C_BYTE_TIMEOUT);
    int code = i2c_write_timeout_us(I2C_PORT, DEVICE_ID, wr, sizeof(wr), false, timeout);
    max30102_status st = reg_write_read_check(code, wr_size);
    return st;
}

static max30102_status reg_read(uint8_t reg, uint8_t *dst, uint8_t len){

    uint timeout = (uint)(2 * I2C_BYTE_TIMEOUT);
    int code1 = i2c_write_timeout_us(I2C_PORT, DEVICE_ID, &reg, 1, true, timeout);
    if (reg_write_read_check(code1, 1) == MAX30102_OK){
        timeout = (uint)((len + 1) * I2C_BYTE_TIMEOUT);
        int code2 = i2c_read_timeout_us(I2C_PORT, DEVICE_ID, dst, len, false, timeout);
        return reg_write_read_check(code2, len);
    }else{
        return reg_write_read_check(code1, 1);
    }
}


max30102_status max30102_init(void){

    max30102_i2c_init();

    max30102_status st = reg_write(REG_MODE_CFG, 0x40); // RESET всех регистров
    if(st != MAX30102_OK){ 
        return st;
    }
    sleep_ms(100);


    st = reg_write(REG_FIFO_CFG, 0x50); // Усреднение по 4 отчета
                                        // При переполнении перезаписывать старые
    if (st != MAX30102_OK){
        return st;
    }

    st = reg_write(REG_LED1_PA, 0x14);  // Ток красного светодиода 4мА
    if (st != MAX30102_OK){
        return st;
    }


    st = reg_write(REG_SPO2_CFG, 0x0C); // 400Гц до усреднения, импульс 69мкс
    if (st != MAX30102_OK){
        return st;
    }

    st = reg_write(REG_MODE_CFG, 0x02); // Запустить измерения
    if (st != MAX30102_OK){
        return st;
    }

    uint8_t dst = 0;
    st = reg_read(REG_PART_ID, &dst, 1);
    if (st == MAX30102_OK && dst != PART_ID){
        return WRONG_PART_ID;
    }else{
        return st;
    }
}

static max30102_status fifo_available(uint8_t *n){
    uint8_t buf[3];
    max30102_status st = reg_read(REG_FIFO_WR, buf, 3);
    if(st == MAX30102_OK){
        uint8_t wr = buf[0];
        uint8_t ovf = buf[1];
        uint8_t rd = buf[2];
        *n = ((wr & 0x1F) - (rd & 0x1F) + 32) % 32;
        return st;
    }else{
        return st;
    }
}


max30102_status max30102_read(uint32_t *out){
        static uint8_t n = 0;
        if (n == 0){
            max30102_status st = fifo_available(&n);
            if (st != MAX30102_OK) return st;
        }
        if (n != 0){
            uint8_t buf[3] = {0};

            max30102_status st = reg_read(REG_FIFO_DATA, buf, 3);
            if (st == MAX30102_OK){
                *out = ((uint32_t)(buf[0] & 0x03) << 16) | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2]); 
                n--;
            }
            return st;
        }else{
            return MAX30102_FIFO_NO_DATA;
        }
}
    



 


