#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#define DC_BUFF_LENGTH 10
#define CONTACT_DC_ON_MAX 145000
#define CONTACT_DC_ON_MIN 120000

static int32_t dc_window[DC_BUFF_LENGTH] = {0};
static int32_t win_index = 0;
static int32_t win_sum = 0;

void contact_reset(void){
    for (uint16_t i = 0; i < DC_BUFF_LENGTH; i++){
        dc_window[i] = 0;
    }
    win_index = 0;
    win_sum = 0;
}


static int32_t dc_from_window(uint32_t raw){
    int32_t i = win_index % DC_BUFF_LENGTH;
    win_sum -= dc_window[i];
    dc_window[i] = (int32_t)raw;
    win_sum += dc_window[i];
    win_index++;
    int32_t dc = 0;
    if (win_index < DC_BUFF_LENGTH){
        dc = win_sum / win_index;
    }else{
        dc = win_sum / DC_BUFF_LENGTH;
    } 
    return dc;
}




bool view_contact(uint32_t raw){
    int32_t dc = dc_from_window(raw);
    if (dc > CONTACT_DC_ON_MIN && dc < CONTACT_DC_ON_MAX){
        return true;
    }else{
        return false;
    }
}