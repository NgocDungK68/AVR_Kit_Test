#ifndef HUNGET_ADC_H
#define HUNGET_ADC_H

#include <avr/io.h>

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
     * Dùng AVCC làm điện áp tham chiếu ADC.
     * Tức Vref khoảng 5V của kit.
     */
    ADMUX &= ~((1 << REFS1) | (1 << REFS0));
    ADMUX |= (1 << REFS0);

    /*
     * Kết quả căn phải.
     */
    ADMUX &= ~(1 << ADLAR);
}

void ADC_IN(unsigned char channel)
{
    channel &= 0x07;

    /*
     * Giữ lại REFS1, REFS0, ADLAR.
     * Chỉ đổi phần chọn kênh ADC.
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
     * Delay rất nhỏ sau khi đổi kênh ADC.
     */
    for (wait_count = 0; wait_count < 100; wait_count++)
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
     * Đọc bỏ lần đầu sau khi chọn kênh.
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
     * điện áp mV = ADC * 5000 / 1024
     */
    mv = (unsigned long)adc_value * 5000UL;
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
     * Nếu hiển thị dạng x10:
     * 250mV = 25.0°C = 250
     *
     * Vậy voltage_mv chính là temp_x10.
     */
    return voltage_mv;
}

#endif