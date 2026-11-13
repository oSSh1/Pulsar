#pragma once

#include <stdint.h>

void hrv_add_peak(int32_t peak_n);
int32_t get_hr(void);
int32_t get_rmssd(void);