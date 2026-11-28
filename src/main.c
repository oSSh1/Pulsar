#include <stdio.h>
#include "pico/stdlib.h" 
#include "pico/time.h"
#include "max30102.h"
#include "fir.h"
#include "ssf.h"
#include "hrv.h"
#include "contact.h"
#include <inttypes.h>
#include <stdbool.h>

#define MS_WITHOUT_DATA 20 
#define LED_ON_TIME_MS 100



static void sensor_bring_up(void){
    max30102_status st = max30102_init();
    firwin_reset();
    detector_reset();
    hrv_reset();
    contact_reset();

    while (st != MAX30102_OK){
        sleep_ms(1000);
        st = max30102_i2c_bus_recovery();
        if (st == MAX30102_OK){
            st = max30102_init();
        }
        printf("%d\n", st); // Вывод ошибки
    }
}

static void led_on(void){
    gpio_put(PICO_DEFAULT_LED_PIN, 1);
}

static void led_off(void){
    gpio_put(PICO_DEFAULT_LED_PIN, 0);
}



static int64_t led_callback_off(alarm_id_t id, void *user_data){
    (void)id; 
    (void)user_data;
    gpio_put(PICO_DEFAULT_LED_PIN, 0);

    return 0;
}

volatile bool ready = false;

void sensor_callback_irq(uint gpio, uint32_t events){

    ready = true;
}

/* --- основной цикл ---------------------------------------------------- */
 
int main(void) {
    stdio_init_all();
    sleep_ms(3000);                         /* дать USB подняться */

    gpio_init(PICO_DEFAULT_LED_PIN);        /* светодиод для мигания на удары */
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_init(PIN_INT);                     /* Настройка прерывания */
    gpio_pull_up(PIN_INT);
    gpio_set_irq_enabled_with_callback(PIN_INT, GPIO_IRQ_EDGE_FALL, true, sensor_callback_irq);

    sensor_bring_up();

    uint32_t n = 0;
    uint64_t last_ok_time = to_ms_64_since_boot(get_absolute_time());
    bool cont_prev = false; 

    
    while (true) {
        if (ready){
           ready = false;
            bool read = true;
            while (read){
                uint32_t raw = 0;
                max30102_status st = max30102_read(&raw);
                if (st == MAX30102_OK){
                    bool cont = view_contact(raw);
                    last_ok_time = to_ms_64_since_boot(get_absolute_time());

                    if (cont && !cont_prev){
                            firwin_reset();
                            detector_reset();
                            hrv_reset();
                            n = 0;
                    }

                    cont_prev = cont;
                    if (cont){
                        int32_t value = firwin_value(raw);
                        int32_t ssf = get_ssf(-value);
                        int32_t peak_n = peak_detector(ssf, n);
                        if(peak_n != 0){

                            led_on();
                            if(add_alarm_in_ms(LED_ON_TIME_MS, led_callback_off, NULL, true) < 0) led_off();

                            hrv_add_peak(peak_n);
                            int32_t hr = get_hr();
                            int32_t rmssd = get_rmssd();
                            printf("HR: %"PRId32", RMSSD: %"PRId32"\n", hr, rmssd);
                        }
                        n++;
                    }
                }else{
                    read = false;
                    if (st != MAX30102_FIFO_NO_DATA){
                        printf("%d\n", st); // Вывод ошибки
                        n = 0;
                        sensor_bring_up();
                        cont_prev = false;
                    }
                    last_ok_time = to_ms_64_since_boot(get_absolute_time());
                }
            }
        }else{
            uint64_t now_ms = to_ms_64_since_boot(get_absolute_time());
            if (now_ms - last_ok_time > MS_WITHOUT_DATA){
                n = 0;
                sensor_bring_up();
                last_ok_time = to_ms_64_since_boot(get_absolute_time());
                cont_prev = false;
            }
        }
    }
}
