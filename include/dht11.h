#ifndef DHT11_H
#define DHT11_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

/*
 * DHT11 DATA dùng PB4.
 * DHT11 DAT -> J4 PB4.
 */
#define DHT11_DDR      DDRB
#define DHT11_PORT     PORTB
#define DHT11_PIN_REG  PINB
#define DHT11_BIT      PB4

#define DHT11_OK                 0
#define DHT11_ERROR_RESPONSE     1
#define DHT11_ERROR_DATA_TIMEOUT 2
#define DHT11_ERROR_CHECKSUM     3

static void DHT11_LINE_LOW(void)
{
    DHT11_DDR |= (1 << DHT11_BIT);
    DHT11_PORT &= ~(1 << DHT11_BIT);
}

static void DHT11_LINE_RELEASE(void)
{
    DHT11_DDR &= ~(1 << DHT11_BIT);
    DHT11_PORT |= (1 << DHT11_BIT);
}

static unsigned char DHT11_READ_PIN(void)
{
    if (DHT11_PIN_REG & (1 << DHT11_BIT))
    {
        return 1;
    }

    return 0;
}

static unsigned char DHT11_WAIT_LEVEL(unsigned char level, unsigned int timeout_us)
{
    while (timeout_us > 0)
    {
        if (DHT11_READ_PIN() == level)
        {
            return 0;
        }

        _delay_us(1);
        timeout_us--;
    }

    return 1;
}

unsigned char DHT11_READ(
    unsigned char *hum_int,
    unsigned char *hum_dec,
    unsigned char *temp_int,
    unsigned char *temp_dec
)
{
    unsigned char data[5] = {0, 0, 0, 0, 0};
    unsigned char i;
    unsigned char j;
    unsigned char bit_index;
    unsigned char status = DHT11_OK;
    unsigned char checksum;

    cli();

    /*
     * Start signal:
     * MCU kéo DATA xuống thấp khoảng 20ms.
     */
    DHT11_LINE_LOW();
    _delay_ms(20);

    /*
     * Thả DATA.
     */
    DHT11_LINE_RELEASE();
    _delay_us(30);

    /*
     * DHT11 response:
     * Low ~80us, High ~80us.
     */
    if (DHT11_WAIT_LEVEL(0, 120))
    {
        status = DHT11_ERROR_RESPONSE;
        goto DHT11_DONE;
    }

    if (DHT11_WAIT_LEVEL(1, 120))
    {
        status = DHT11_ERROR_RESPONSE;
        goto DHT11_DONE;
    }

    if (DHT11_WAIT_LEVEL(0, 120))
    {
        status = DHT11_ERROR_RESPONSE;
        goto DHT11_DONE;
    }

    /*
     * Đọc 40 bit.
     */
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 8; j++)
        {
            bit_index = (unsigned char)(7 - j);

            /*
             * Chờ xung high của bit.
             */
            if (DHT11_WAIT_LEVEL(1, 100))
            {
                status = DHT11_ERROR_DATA_TIMEOUT;
                goto DHT11_DONE;
            }

            /*
             * Sau 40us:
             * - nếu vẫn high: bit 1
             * - nếu đã low: bit 0
             */
            _delay_us(40);

            if (DHT11_READ_PIN())
            {
                data[i] |= (1 << bit_index);
            }

            /*
             * Chờ hết xung high.
             */
            if (!((i == 4) && (j == 7)))
            {
                if (DHT11_WAIT_LEVEL(0, 100))
                {
                    status = DHT11_ERROR_DATA_TIMEOUT;
                    goto DHT11_DONE;
                }
            }
        }
    }

DHT11_DONE:

    sei();

    if (status != DHT11_OK)
    {
        return status;
    }

    checksum = (unsigned char)(data[0] + data[1] + data[2] + data[3]);

    if (checksum != data[4])
    {
        return DHT11_ERROR_CHECKSUM;
    }

    *hum_int = data[0];
    *hum_dec = data[1];
    *temp_int = data[2];
    *temp_dec = data[3];

    return DHT11_OK;
}

#endif