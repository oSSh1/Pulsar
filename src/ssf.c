#include <stdio.h>
#include "ssf.h"
#include "fir.h"

#define X_LENGTH 5
#define WIN_LENGTH 12
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define WARMUP 200
#define REFR 25

struct detector{
    int32_t spk;
    int32_t npk;
    int32_t ssf_prev;
    int32_t ssf_prev_prev;
    int32_t thr1;
    int32_t thr2;
    uint8_t refr;
    int32_t contender_peak;
    uint32_t contender_peak_n;
    uint32_t rr_buff[8];
    uint32_t rr_sum;
    uint32_t rr_index;
    uint32_t last_peak_n;
};

int32_t get_ssf(int32_t value){
    static int32_t x[X_LENGTH] = {0};
    static int32_t s[WIN_LENGTH] = {0};
    static uint8_t x_count = 0;
    static uint8_t s_count = 0;
    static int32_t ssf = 0;
    if (x_count < X_LENGTH){
        x[x_count] = value; 
        x_count++;
    }else{
        for(uint8_t i = 1; i < X_LENGTH; i++){
            x[i - 1] = x[i];
        }
        x[x_count - 1] = value;
    }

    if(x_count >= X_LENGTH){
        int32_t d = (2 * x[4] + x[3] - x[1] - 2 * x[0]);
        if (s_count < WIN_LENGTH){
            s[s_count] = MAX(0, d);
            ssf += s[s_count];
            s_count++;
        }else{
            ssf -= s[0];
            for(uint8_t i = 1; i < WIN_LENGTH; i++){
            s[i - 1] = s[i];
            }
            s[s_count - 1] = MAX(0, d);
            ssf += s[s_count - 1];
        }
    }
    return ssf;
}

static void update_rr_buff(struct detector *det, uint32_t peak_n){
    if (det->last_peak_n != 0){
        det->rr_sum -= det->rr_buff[(det->rr_index) % 8];
        det->rr_buff[(det->rr_index) % 8] = (peak_n - det->last_peak_n);
        det->rr_sum += (peak_n - det->last_peak_n);
        det->rr_index++;
    }
    det->contender_peak = 0;
    det->contender_peak_n = 0;
    det->last_peak_n = peak_n;
}


int32_t peak_detector(int32_t ssf, uint32_t n){
    int32_t peak_n = 0;
    static struct detector det = {0};
    if(n > BUFF_LENGTH){
        if(n > BUFF_LENGTH + WARMUP){
            det.thr1 = det.npk + ((det.spk - det.npk) / 4); 
            det.thr2 = det.thr1 / 2;
            if (det.refr == 0){
                if ((det.ssf_prev_prev < det.ssf_prev) && (det.ssf_prev >= ssf)){
                    int32_t peak = det.ssf_prev;
                    if(peak > det.thr1){
                        det.spk = (peak / 8) + ((det.spk * 7) / 8);
                        det.refr = REFR;
                        peak_n = n - 1;
                        update_rr_buff(&det, peak_n);
                    }else{
                        det.npk = (peak / 8) + ((det.npk * 7) / 8);
                        if (peak > det.thr2){
                            if (det.contender_peak < peak){
                                det.contender_peak = peak;
                                det.contender_peak_n = n - 1;
                            }
                        }
                    }
                }
            }else{
                det.refr--;
            }
            if (det.rr_index >= 8 && det.last_peak_n != 0 && (n - det.last_peak_n) > (5 * (det.rr_sum / 8) / 3)){
                if (det.contender_peak != 0){
                    peak_n = det.contender_peak_n;
                    det.spk = (det.contender_peak / 4) + (3 * det.spk / 4);
                    update_rr_buff(&det, peak_n);
                }
            }
        }else{
            if (n < BUFF_LENGTH + WARMUP){
                det.spk = MAX(det.spk, ssf);
                det.npk += ssf;
            }else{
                det.spk = MAX(det.spk, ssf) / 3;
                det.npk = (det.npk + ssf) / WARMUP;
            }
        }
    }

    det.ssf_prev_prev = det.ssf_prev;
    det.ssf_prev = ssf;

    return peak_n;
}