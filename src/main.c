#include <stdio.h>
#include "pico/stdlib.h" 
#include "max30102.h"
#include "fir.h"
#include "ssf.h"
#include "hrv.h"
#include <inttypes.h>

/* --- основной цикл ---------------------------------------------------- */
 
int main(void) {
    stdio_init_all();
    sleep_ms(3000);                         /* дать USB подняться */
    
    max30102_status st = max30102_init();
    if (st != MAX30102_OK) {
        while (true){
            printf("%d\n", st);
            sleep_ms(1000);
        }
    }

    uint32_t n = 0;
    while (true) {
        uint32_t raw = 0;
        if (max30102_read(&raw) == MAX30102_OK){
            int32_t value = firwin_value(raw);
            int32_t ssf = get_ssf(-value);
            int32_t peak_n = peak_detector(ssf, n);
            if(peak_n != 0){
                hrv_add_peak(peak_n);
                int32_t hr = get_hr();
                int32_t rmssd = get_rmssd();
                printf("HR: %"PRId32", RMSSD: %"PRId32"\n", hr, rmssd);
            }
            n++;
        }
        sleep_ms(5);
    }
}