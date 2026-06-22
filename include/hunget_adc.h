#ifndef HUNGET_ADC_H
#define HUNGET_ADC_H

#include <avr/io.h>

/*
 * Điện áp tham chiếu ADC.
 * Nếu đo J8 5V ra đúng 5.00V thì để 5000.
 * Nếu đo ra 4.95V thì sửa thành 4950.
 */
#ifndef ADC_VREF_MV
#define ADC_VREF_MV 5000UL
#endif

void ADC_PRES(unsigned char prescaler);
void ADC_AVCC(void);
void ADC_IN(unsigned char channel);
void ADC_STA_CONVERT(void);
unsigned int ADC_READ(unsigned char channel);
unsigned int ADC_READ_AVG(unsigned char channel, unsigned char sample_count);
unsigned int ADC_TO_MV(unsigned int adc_value);
unsigned int ADC_LM35_TEMP_X10(unsigned int adc_value);

void ADC_PRES(unsigned char prescaler)
{
    ADCSRA |= (1 << ADEN);

    ADCSRA &= ~((1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0));

    switch (prescaler)
    {
        case 2:
            ADCSRA |= (1 << ADPS0);
            break;

        case 4:
            ADCSRA |= (1 << ADPS1);
            break;

        case 8:
            ADCSRA |= (1 << ADPS1) | (1 << ADPS0);
            break;

        case 16:
            ADCSRA |= (1 << ADPS2);
            break;

        case 32:
            ADCSRA |= (1 << ADPS2) | (1 << ADPS0);
            break;

        case 64:
            ADCSRA |= (1 << ADPS2) | (1 << ADPS1);
            break;

        case 128:
        default:
            ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
            break;
    }
}

void ADC_AVCC(void)
{
    /*
     * REFS1 = 0, REFS0 = 1
     * Vref = AVCC, tức dùng nguồn 5V của kit làm tham chiếu.
     */
    ADMUX &= ~((1 << REFS1) | (1 << REFS0));
    ADMUX |= (1 << REFS0);

    /*
     * ADLAR = 0, kết quả ADC căn phải.
     */
    ADMUX &= ~(1 << ADLAR);
}

void ADC_IN(unsigned char channel)
{
    channel &= 0x07;

    /*
     * Giữ lại REFS1, REFS0, ADLAR.
     * Chỉ thay phần chọn kênh ADC.
     */
    ADMUX &= 0xE0;
    ADMUX |= channel;
}

void ADC_STA_CONVERT(void)
{
    ADCSRA |= (1 << ADSC);

    while (ADCSRA & (1 << ADSC))
    {
    }
}

unsigned int ADC_READ(unsigned char channel)
{
    volatile unsigned int wait_count;

    ADC_IN(channel);

    /*
     * Chờ rất ngắn sau khi chọn kênh ADC.
     */
    for (wait_count = 0; wait_count < 200; wait_count++)
    {
        __asm__ __volatile__("nop");
    }

    ADC_STA_CONVERT();

    return ADC;
}

unsigned int ADC_READ_AVG(unsigned char channel, unsigned char sample_count)
{
    unsigned long sum = 0;
    unsigned char i;

    if (sample_count == 0)
    {
        sample_count = 1;
    }

    /*
     * Đọc bỏ lần đầu để ADC ổn định sau khi chọn kênh.
     */
    ADC_READ(channel);

    for (i = 0; i < sample_count; i++)
    {
        sum += ADC_READ(channel);
    }

    return (unsigned int)(sum / sample_count);
}

unsigned int ADC_TO_MV(unsigned int adc_value)
{
    unsigned long mv;

    /*
     * ADC 10-bit:
     * mV = ADC * Vref / 1024
     */
    mv = (unsigned long)adc_value * ADC_VREF_MV;
    mv = mv + 512UL;
    mv = mv / 1024UL;

    return (unsigned int)mv;
}

unsigned int ADC_LM35_TEMP_X10(unsigned int adc_value)
{
    unsigned int voltage_mv;

    voltage_mv = ADC_TO_MV(adc_value);

    /*
     * LM35:
     * 10mV = 1°C
     *
     * Nếu nhiệt độ dạng x10:
     * 25.0°C = 250
     *
     * 250mV từ LM35 tương ứng 25.0°C.
     * Vì vậy voltage_mv chính là temp_x10.
     */
    return voltage_mv;
}

#endif