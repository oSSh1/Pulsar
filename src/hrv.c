#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include "hrv.h"

#define RR_WIN_LENGTH 60
#define RR_MIN 300
#define RR_MAX 2000
#define ABS_MINUS(x, y) ((x) > (y) ? ((x) - (y)) : ((y) - (x)))
#define PEAKS_TO_RESET 5 
#define MS_MULTIPLIER 10
#define MIN_RR_TO_HR 8
#define MIN_PAIRS_TO_RMSSD 15
#define MIN(x, y) ((x) > (y) ? (y) : (x))


static int32_t rr[RR_WIN_LENGTH] = {0};
static int32_t rr_index = 0;
static bool ok[RR_WIN_LENGTH] = {false};
static int32_t last_n = 0;
static int32_t ref = 0;
static int32_t bad_peaks = 0;


void hrv_reset(void){
    for (uint16_t i = 0; i < RR_WIN_LENGTH; i++){
        rr[i] = 0;
    }
    for (uint16_t i = 0; i < RR_WIN_LENGTH; i++){
        ok[i] = false;
    }
    rr_index = 0;
    last_n = 0;
    ref = 0;
    bad_peaks = 0;

}



void hrv_add_peak(int32_t peak_n){
    int32_t t = rr_index % RR_WIN_LENGTH;
    int32_t cur = (peak_n - last_n) * MS_MULTIPLIER;
    rr[t] = cur;
    if (rr_index == 0){
        if (last_n != 0 && cur < RR_MAX && cur > RR_MIN){
            ref = cur;
            ok[t] = true;
            rr_index++;
        }
    }else{ 
        if (cur < RR_MAX && cur > RR_MIN && ABS_MINUS(cur, ref) <= (ref / 4)){
                ok[t] = true;
                bad_peaks = 0;
                ref = cur;
        }else{
            ok[t] = false;
            bad_peaks++;
            if (bad_peaks >= PEAKS_TO_RESET){
                if (cur < RR_MAX && cur > RR_MIN){
                    ref = cur;
                }
                bad_peaks = 0;
            }
       }
       rr_index++;
    }
    last_n = peak_n;
}




int32_t get_hr(void){
    int32_t rr_sum = 0;
    int32_t ok_count = 0;
    if(rr_index >= MIN_RR_TO_HR){
        for(int32_t i = 0; i < MIN(rr_index, RR_WIN_LENGTH); i++){
            if(ok[i]){
                rr_sum += rr[i];
                ok_count++;
            }
        }
        if (ok_count >= MIN_RR_TO_HR){
            return (60000 * ok_count) / rr_sum;
        }
    }
    return 0;
}

int32_t get_rmssd(void){
    if(rr_index >= RR_WIN_LENGTH){
        int32_t t = 0;
        int32_t t_prev = 0;
        int32_t rr_diff_sum = 0;
        int32_t ok_pairs_count = 0;
        for (int32_t i = 0; i < (RR_WIN_LENGTH - 1); i++){
            t = (rr_index - i - 1) % RR_WIN_LENGTH;
            t_prev = (rr_index - i - 2) % RR_WIN_LENGTH;
            if (ok[t] && ok[t_prev]){
                ok_pairs_count++;
                rr_diff_sum += (rr[t] - rr[t_prev]) * (rr[t] - rr[t_prev]);
            }
        }
        if(ok_pairs_count >= MIN_PAIRS_TO_RMSSD){
            int32_t rmssd = sqrtf(rr_diff_sum / (ok_pairs_count));
            return rmssd;
        }
    }
    return 0;
}

