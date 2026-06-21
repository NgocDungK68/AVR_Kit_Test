#include <avr/io.h>

#define FRE 8

#include "hunget_adc.h"
#include "hunget_lcd.h"
#include "thu_vien_rieng.h"

int main(void)
{
    INIT();

    LM35_2_LCD();

    return 0;
}