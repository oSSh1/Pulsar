#pragma once

#include <stdint.h>

void detector_reset(void);
int32_t get_ssf(int32_t value);
int32_t peak_detector(int32_t ssf, uint32_t n);
