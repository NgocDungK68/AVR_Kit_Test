#ifndef THU_VIEN_RIENG_H
#define THU_VIEN_RIENG_H

#include <avr/io.h>
#include "hunget_lcd.h"
#include "dht11.h"

#ifndef FRE
#define FRE 8
#endif

void INIT(void);
void DHT11_2_LCD(void);
void LCD_CLEAR_LINE(unsigned char row);
void LCD_PRINT_2DIGIT(unsigned char value);
void DELAY_MS(unsigned int mili_count);

void INIT(void)
{
    /*
     * LCD dùng:
     * PD5, PD6, PD7
     * PC4, PC5, PC6, PC7
     */
    DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
    PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD7));

    DDRC |= 0xF0;
    PORTC |= 0x0F;

    /*
     * DHT11 DATA dùng PB4.
     * Ban đầu để input + pull-up.
     */
    DHT11_DDR &= ~(1 << DHT11_BIT);
    DHT11_PORT |= (1 << DHT11_BIT);
}

void LCD_CLEAR_LINE(unsigned char row)
{
    LCD4_CUR_GOTO(row, 0);
    LCD4_OUT_STR("                ");
}

void LCD_PRINT_2DIGIT(unsigned char value)
{
    if (value < 10)
    {
        LCD4_OUT_DATA('0');
        LCD4_OUT_DEC(value, 1);
    }
    else
    {
        LCD4_OUT_DEC(value, 2);
    }
}

void DHT11_2_LCD(void)
{
    unsigned char hum_i;
    unsigned char hum_d;
    unsigned char temp_i;
    unsigned char temp_d;
    unsigned char status;

    LCD4_INIT(0, 0);
    LCD4_OUT_CMD(0x01);

    LCD4_CUR_GOTO(1, 0);
    LCD4_OUT_STR("DHT11 TEST");

    LCD4_CUR_GOTO(2, 0);
    LCD4_OUT_STR("Starting...");

    /*
     * DHT11 cần một chút thời gian ổn định sau khi cấp nguồn.
     */
    DELAY_MS(2000);

    for (;;)
    {
        status = DHT11_READ(&hum_i, &hum_d, &temp_i, &temp_d);

        if (status == DHT11_OK)
        {
            /*
             * Dòng 1: nhiệt độ.
             */
            LCD_CLEAR_LINE(1);
            LCD4_CUR_GOTO(1, 0);
            LCD4_OUT_STR("Temp: ");
            LCD_PRINT_2DIGIT(temp_i);
            LCD4_OUT_DATA('.');
            LCD4_OUT_DEC(temp_d, 1);
            LCD4_OUT_DATA('C');

            /*
             * Dòng 2: độ ẩm.
             */
            LCD_CLEAR_LINE(2);
            LCD4_CUR_GOTO(2, 0);
            LCD4_OUT_STR("Hum : ");
            LCD_PRINT_2DIGIT(hum_i);
            LCD4_OUT_DATA('.');
            LCD4_OUT_DEC(hum_d, 1);
            LCD4_OUT_DATA('%');
        }
        else
        {
            LCD_CLEAR_LINE(1);
            LCD4_CUR_GOTO(1, 0);
            LCD4_OUT_STR("DHT11 ERROR");

            LCD_CLEAR_LINE(2);
            LCD4_CUR_GOTO(2, 0);
            LCD4_OUT_STR("Code: ");
            LCD4_OUT_DEC(status, 1);
        }

        /*
         * DHT11 không nên đọc quá nhanh.
         * Đọc mỗi 2 giây là ổn.
         */
        DELAY_MS(2000);
    }
}

void DELAY_MS(unsigned int mili_count)
{
    unsigned int i;
    unsigned int j;

    mili_count = mili_count * FRE;

    for (i = 0; i < mili_count; i++)
    {
        for (j = 0; j < 53; j++)
        {
            __asm__ __volatile__("nop");
        }
    }
}

#endif