#pragma once

#include <stdint.h>

typedef enum {
    MAX30102_OK = 0,
    MAX30102_IS_NOT_CONNECTED,
    WRITE_DATA_LOST,
    WRONG_PART_ID,
    REG_RESET_FAILURE,
    REG_READ_WRITE_TIMEOUT,
    MAX30102_UNKNOWN_ERROR,
    MAX30102_FIFO_NO_DATA
} max30102_status;


max30102_status max30102_init(void);
max30102_status max30102_read(uint32_t *out);

