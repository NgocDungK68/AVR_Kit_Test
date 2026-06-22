#include <avr/io.h>

#define FRE 8

#include "hunget_lcd.h"
#include "dht11.h"
#include "thu_vien_rieng.h"

int main(void)
{
    INIT();

    DHT11_2_LCD();

    return 0;
}