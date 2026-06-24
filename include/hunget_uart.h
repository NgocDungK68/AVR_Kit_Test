#ifndef HUNGET_UART_H
#define HUNGET_UART_H

#include <avr/io.h>

/*
 * UART ATmega16:
 * PD0 = RXD
 * PD1 = TXD
 *
 * Với F_CPU = 8 MHz, baudrate = 9600:
 * UBRR = 51
 */

void UART_INIT(unsigned int ubrr, unsigned char data_bits, unsigned char parity, unsigned char stop_bits);
void UART_TRAN_BYTE(unsigned char data);
void UART_TRAN_STR(const char *str);

void UART_INIT(unsigned int ubrr, unsigned char data_bits, unsigned char parity, unsigned char stop_bits)
{
    unsigned char ucsrc_value;

    /*
     * Thiết lập baudrate.
     * UBRR = 51 tương ứng 9600 bps khi F_CPU = 8 MHz.
     */
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    /*
     * Không dùng double speed.
     */
    UCSRA = 0x00;

    /*
     * Bật bộ truyền TX và bộ nhận RX.
     */
    UCSRB = (1 << RXEN) | (1 << TXEN);

    /*
     * URSEL = 1 để ghi vào UCSRC.
     */
    ucsrc_value = (1 << URSEL);

    /*
     * Chọn số bit dữ liệu.
     * Bài này dùng 8 bit.
     */
    switch (data_bits)
    {
        case 5:
            break;

        case 6:
            ucsrc_value |= (1 << UCSZ0);
            break;

        case 7:
            ucsrc_value |= (1 << UCSZ1);
            break;

        case 8:
        default:
            ucsrc_value |= (1 << UCSZ1) | (1 << UCSZ0);
            break;
    }

    /*
     * Chọn parity.
     * parity = 0: không parity
     * parity = 1: even parity
     * parity = 2: odd parity
     */
    if (parity == 1)
    {
        ucsrc_value |= (1 << UPM1);
    }
    else if (parity == 2)
    {
        ucsrc_value |= (1 << UPM1) | (1 << UPM0);
    }

    /*
     * Chọn stop bit.
     * stop_bits = 1 hoặc 2.
     */
    if (stop_bits == 2)
    {
        ucsrc_value |= (1 << USBS);
    }

    UCSRC = ucsrc_value;
}

void UART_TRAN_BYTE(unsigned char data)
{
    /*
     * Chờ thanh ghi truyền rỗng.
     */
    while (!(UCSRA & (1 << UDRE)))
    {
    }

    /*
     * Gửi 1 byte ra UART.
     */
    UDR = data;
}

void UART_TRAN_STR(const char *str)
{
    while (*str)
    {
        UART_TRAN_BYTE((unsigned char)(*str));
        str++;
    }
}

#endif