#include <stdio.h>
#include "pico/stdlib.h" 
#include "max30102.h"
#include "fir.h"
#include "ssf.h"
#include "hrv.h"
#include <inttypes.h>

static void sensor_bring_up(void){
    max30102_status st = max30102_init();

    while (st != MAX30102_OK){

        if (max30102_i2c_bus_recovery() == MAX30102_OK){
            st = max30102_init();
            printf("%d\n", st);
        }else{
            printf("MAX30102_RECOVERY_ERROR\n");
        }

        sleep_ms(1000);
    }
}


/* --- основной цикл ---------------------------------------------------- */
 
int main(void) {
    stdio_init_all();
    sleep_ms(3000);                         /* дать USB подняться */

    sensor_bring_up();

    uint32_t n = 0;
    while (true) {
        uint32_t raw = 0;
        max30102_status st = max30102_read(&raw);
        if (st == MAX30102_OK){
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
        }else{
            if (st == MAX30102_READ_WRITE_TIMEOUT){
                sensor_bring_up();
            }
        }
        sleep_ms(5);
    }
}