#ifndef DHT11_H
#define DHT11_H

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdint.h>

/*
 * DHT11 DATA dùng PB4.
 * Nếu bạn đổi sang chân khác thì sửa DHT11_BIT.
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
    /*
     * Thả chân DATA.
     * Bật pull-up nội để đường DATA có mức 1 khi rảnh.
     * Module DHT11 thường đã có trở kéo lên sẵn, nhưng bật thêm vẫn ổn.
     */
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

/*
 * Chờ chân DATA đạt mức level trong timeout_us.
 * level = 0: chờ xuống thấp
 * level = 1: chờ lên cao
 *
 * Trả về:
 * 0: thành công
 * 1: timeout
 */
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

/*
 * Đọc DHT11.
 *
 * hum_int: phần nguyên độ ẩm
 * hum_dec: phần thập phân độ ẩm, DHT11 thường là 0
 * temp_int: phần nguyên nhiệt độ
 * temp_dec: phần thập phân nhiệt độ, DHT11 thường là 0
 */
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

    /*
     * DHT11 cần khoảng 1 giây giữa hai lần đọc.
     * Việc delay lớn nằm ở hàm gọi ngoài.
     */

    cli();

    /*
     * Tín hiệu start:
     * MCU kéo DATA xuống thấp ít nhất 18ms.
     */
    DHT11_LINE_LOW();
    _delay_ms(20);

    /*
     * Thả DATA lên cao, rồi chuyển sang input để DHT11 trả lời.
     */
    DHT11_LINE_RELEASE();
    _delay_us(30);

    /*
     * DHT11 response:
     * Low khoảng 80us, high khoảng 80us.
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
     * Đọc 40 bit dữ liệu.
     * Mỗi bit:
     * - bắt đầu bằng low khoảng 50us
     * - high ngắn khoảng 26-28us là bit 0
     * - high dài khoảng 70us là bit 1
     */
    for (i = 0; i < 5; i++)
    {
        for (j = 0; j < 8; j++)
        {
            bit_index = (unsigned char)(7 - j);

            /*
             * Chờ bắt đầu xung high của bit.
             */
            if (DHT11_WAIT_LEVEL(1, 100))
            {
                status = DHT11_ERROR_DATA_TIMEOUT;
                goto DHT11_DONE;
            }

            /*
             * Sau khoảng 40us:
             * - nếu vẫn high: bit 1
             * - nếu đã low: bit 0
             */
            _delay_us(40);

            if (DHT11_READ_PIN())
            {
                data[i] |= (1 << bit_index);
            }

            /*
             * Nếu đang là bit 1, chờ hết xung high.
             * Nếu là bit 0, chân đã low sẵn nên lệnh này trả về ngay.
             * Riêng bit cuối cùng không cần chờ.
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