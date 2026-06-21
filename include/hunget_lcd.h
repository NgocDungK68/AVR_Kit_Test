#ifndef HUNGET_LCD_H
#define HUNGET_LCD_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>

/*
 * Kết nối LCD 1602 trên kit:
 * RS -> PD6
 * RW -> PD5
 * E  -> PD7
 * D4 -> PC4
 * D5 -> PC5
 * D6 -> PC6
 * D7 -> PC7
 */

#define LCD4_RS        PD6
#define LCD4_RW        PD5
#define LCD4_E         PD7

#define LCD4_CTRL_DDR  DDRD
#define LCD4_CTRL_PORT PORTD

#define LCD4_DATA_DDR  DDRC
#define LCD4_DATA_PORT PORTC

void LCD4_OUT_CMD(unsigned char cmd);
void LCD4_OUT_DATA(unsigned char data);
void LCD4_INIT(unsigned char cursor, unsigned char blink);
void LCD4_CUR_GOTO(unsigned char row, unsigned char col);
void LCD4_OUT_STR(const char *str);
void LCD4_OUT_DEC(unsigned int num, unsigned char digit_count);

static void LCD4_ENABLE_PULSE(void)
{
    LCD4_CTRL_PORT |= (1 << LCD4_E);
    _delay_us(2);

    LCD4_CTRL_PORT &= ~(1 << LCD4_E);
    _delay_us(50);
}

static void LCD4_OUT_NIBBLE(unsigned char nibble)
{
    LCD4_DATA_PORT = (LCD4_DATA_PORT & 0x0F) | ((nibble & 0x0F) << 4);
    LCD4_ENABLE_PULSE();
}

void LCD4_OUT_CMD(unsigned char cmd)
{
    LCD4_CTRL_PORT &= ~(1 << LCD4_RS);
    LCD4_CTRL_PORT &= ~(1 << LCD4_RW);

    LCD4_OUT_NIBBLE(cmd >> 4);
    LCD4_OUT_NIBBLE(cmd);

    if ((cmd == 0x01) || (cmd == 0x02))
    {
        _delay_ms(2);
    }
    else
    {
        _delay_us(50);
    }
}

void LCD4_OUT_DATA(unsigned char data)
{
    LCD4_CTRL_PORT |= (1 << LCD4_RS);
    LCD4_CTRL_PORT &= ~(1 << LCD4_RW);

    LCD4_OUT_NIBBLE(data >> 4);
    LCD4_OUT_NIBBLE(data);

    _delay_us(50);
}

void LCD4_INIT(unsigned char cursor, unsigned char blink)
{
    unsigned char display_control;

    LCD4_CTRL_DDR |= (1 << LCD4_RS) | (1 << LCD4_RW) | (1 << LCD4_E);
    LCD4_DATA_DDR |= 0xF0;

    LCD4_CTRL_PORT &= ~((1 << LCD4_RS) | (1 << LCD4_RW) | (1 << LCD4_E));

    _delay_ms(20);

    LCD4_OUT_NIBBLE(0x03);
    _delay_ms(5);

    LCD4_OUT_NIBBLE(0x03);
    _delay_us(150);

    LCD4_OUT_NIBBLE(0x03);
    _delay_us(150);

    LCD4_OUT_NIBBLE(0x02);
    _delay_us(150);

    LCD4_OUT_CMD(0x28);

    display_control = 0x0C;

    if (cursor)
    {
        display_control |= 0x02;
    }

    if (blink)
    {
        display_control |= 0x01;
    }

    LCD4_OUT_CMD(display_control);
    LCD4_OUT_CMD(0x01);
    LCD4_OUT_CMD(0x06);
}

void LCD4_CUR_GOTO(unsigned char row, unsigned char col)
{
    unsigned char address;

    if (row == 1)
    {
        address = 0x00 + col;
    }
    else
    {
        address = 0x40 + col;
    }

    LCD4_OUT_CMD(0x80 | address);
}

void LCD4_OUT_STR(const char *str)
{
    while (*str)
    {
        LCD4_OUT_DATA((unsigned char)(*str));
        str++;
    }
}

void LCD4_OUT_DEC(unsigned int num, unsigned char digit_count)
{
    unsigned int divisor = 1;
    unsigned char i;
    unsigned char digit;

    if (digit_count == 0)
    {
        return;
    }

    for (i = 1; i < digit_count; i++)
    {
        divisor *= 10;
    }

    while (divisor > 0)
    {
        digit = (unsigned char)(num / divisor);
        LCD4_OUT_DATA((unsigned char)('0' + digit));

        num = num % divisor;
        divisor = divisor / 10;
    }
}

#endif