#ifndef THU_VIEN_RIENG_H
#define THU_VIEN_RIENG_H

#include <avr/io.h>

/*
 * Dùng PA1 / ADC1 để đọc LM35.
 * Không dùng PA0 vì PA0 trên kit có biến trở VR1.
 */
#define LM35_ADC_CHANNEL 1

/*
 * Nếu nhiệt độ bị lệch, chỉnh offset ở đây.
 * Đơn vị: 0.1°C.
 *
 * Ví dụ:
 * LCD báo 32.0°C, nhiệt kế thật là 29.0°C
 * => cao hơn 3.0°C
 * => đặt -30
 */
#define TEMP_CAL_OFFSET_X10 0

void INIT(void);
void LM35_2_LCD(void);
void LCD_CLEAR_LINE(unsigned char row);
void LCD_PRINT_TEMP_X10(unsigned int temp_x10);
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
    PORTC &= 0x0F;

    /*
     * Port A làm input ADC.
     * Không bật pull-up để đọc analog đúng.
     */
    DDRA = 0x00;
    PORTA = 0x00;

    /*
     * Khởi tạo ADC bằng thư viện hunget_adc.h
     */
    ADC_PRES(128);
    ADC_AVCC();
    ADC_IN(LM35_ADC_CHANNEL);
}

void LCD_CLEAR_LINE(unsigned char row)
{
    LCD4_CUR_GOTO(row, 0);
    LCD4_OUT_STR("                ");
}

void LCD_PRINT_TEMP_X10(unsigned int temp_x10)
{
    unsigned int integer_part;
    unsigned int decimal_part;

    integer_part = temp_x10 / 10;
    decimal_part = temp_x10 % 10;

    if (integer_part < 10)
    {
        LCD4_OUT_DATA('0');
        LCD4_OUT_DEC(integer_part, 1);
    }
    else if (integer_part < 100)
    {
        LCD4_OUT_DEC(integer_part, 2);
    }
    else
    {
        LCD4_OUT_DEC(integer_part, 3);
    }

    LCD4_OUT_DATA('.');
    LCD4_OUT_DEC(decimal_part, 1);
    LCD4_OUT_DATA('C');
}

void LM35_2_LCD(void)
{
    unsigned int adc_value;
    unsigned int temp_x10;
    int temp_calibrated;

    LCD4_INIT(0, 0);
    LCD4_OUT_CMD(0x01);

    LCD4_CUR_GOTO(1, 0);
    LCD4_OUT_STR("LM35 TEMP TEST");

    LCD4_CUR_GOTO(2, 0);
    LCD4_OUT_STR("Temp: --.-C");

    DELAY_MS(1000);

    for (;;)
    {
        /*
         * Đọc trung bình 32 mẫu để giảm nhiễu.
         */
        adc_value = ADC_READ_AVG(LM35_ADC_CHANNEL, 32);

        /*
         * Đổi ADC sang nhiệt độ dạng x10.
         * Ví dụ:
         * 253 nghĩa là 25.3°C.
         */
        temp_x10 = ADC_LM35_TEMP_X10(adc_value);

        /*
         * Bù sai số nếu cần.
         */
        temp_calibrated = (int)temp_x10 + TEMP_CAL_OFFSET_X10;

        if (temp_calibrated < 0)
        {
            temp_calibrated = 0;
        }

        if (temp_calibrated > 999)
        {
            temp_calibrated = 999;
        }

        LCD_CLEAR_LINE(1);
        LCD4_CUR_GOTO(1, 0);
        LCD4_OUT_STR("ADC1:");
        LCD4_OUT_DEC(adc_value, 4);

        LCD_CLEAR_LINE(2);
        LCD4_CUR_GOTO(2, 0);
        LCD4_OUT_STR("Temp: ");
        LCD_PRINT_TEMP_X10((unsigned int)temp_calibrated);

        DELAY_MS(500);
    }
}

void DELAY_MS(unsigned int mili_count)
{
    unsigned int i, j;

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