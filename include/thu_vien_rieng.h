#ifndef THU_VIEN_RIENG_H
#define THU_VIEN_RIENG_H

#include <avr/io.h>
#include "hunget_lcd.h"
#include "dht11.h"

#ifndef FRE
#define FRE 8
#endif

/*
 * push_button được khai báo trong main.c
 */
extern unsigned char push_button;

void INIT_LED_DEMO(void);
void INIT_BUTTON_LED(void);
void INIT_DHT_LCD(void);
void INIT_DHT_LED_NO_LCD(void);

void LED_DEMO_APP(void);
void PB_2_LED_APP(void);
void DHT11_LCD_APP(void);
void DHT11_AND_LED_APP(void);

void LED_DEMO_STEP(void);
void LED7_OUT(unsigned char num);
unsigned char PB_CHECK(void);

void LCD_CLEAR_LINE(unsigned char row);
void LCD_PRINT_2DIGIT(unsigned char value);
void DELAY_MS(unsigned int mili_count);

void INIT_UART_USB_LCD(void);
void UART_USB_LCD_APP(void);
void LCD4_DIS_SHIFT(unsigned char lcd4_direct, unsigned char lcd4_step);

/*
 * INIT_LED_DEMO()
 * Dùng khi chạy LED đơn + LED 7 thanh.
 * Cắm JP1, JP2.
 * Không dùng LCD.
 */
void INIT_LED_DEMO(void)
{
    /*
     * Port D điều khiển dãy LED đơn.
     * LED đơn active-low: 0 sáng, 1 tắt.
     */
    DDRD = 0xFF;
    PORTD = 0xFF;

    /*
     * Port C điều khiển LED 7 thanh.
     * LED 7 thanh active-low: 0 sáng, 1 tắt.
     */
    DDRC = 0xFF;
    PORTC = 0xFF;
}

/*
 * INIT_BUTTON_LED()
 * Bài bấm nút hiện LED theo tài liệu mục 3.3.
 *
 * Cắm JP1, JP2.
 * Không dùng LCD.
 *
 * Phím trên kit:
 * PB1 trên board -> chân PB0 của ATmega16
 * PB2 trên board -> chân PB1
 * PB3 trên board -> chân PB2
 * PB4 trên board -> chân PB3
 */
void INIT_BUTTON_LED(void)
{
    /*
     * Port D: dãy LED đơn.
     */
    DDRD = 0xFF;
    PORTD = 0xFF;

    /*
     * Port C: LED 7 thanh.
     */
    DDRC = 0xFF;
    PORTC = 0xFF;

    /*
     * PB0-PB3 là input để đọc 4 nút.
     * Bật pull-up nội:
     * - Không bấm: đọc 1
     * - Bấm: bị kéo xuống GND, đọc 0
     */
    DDRB &= ~(0x0F);
    PORTB |= 0x0F;

    push_button = 0;

    LED7_OUT(0);
    PORTD = 0xFF;
}

/*
 * INIT_DHT_LCD()
 * Dùng khi chạy DHT11 + LCD.
 * Tháo JP1, JP2.
 */
void INIT_DHT_LCD(void)
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
     * Input + pull-up.
     */
    DHT11_DDR &= ~(1 << DHT11_BIT);
    DHT11_PORT |= (1 << DHT11_BIT);
}

/*
 * INIT_DHT_LED_NO_LCD()
 * Dùng khi muốn DHT11 vẫn đọc, LED đơn + LED 7 thanh vẫn chạy.
 * Không dùng LCD.
 */
void INIT_DHT_LED_NO_LCD(void)
{
    INIT_LED_DEMO();

    DHT11_DDR &= ~(1 << DHT11_BIT);
    DHT11_PORT |= (1 << DHT11_BIT);
}

/*
 * LED_DEMO_APP()
 * Bài LED theo tài liệu mục 3.2:
 * - LED đơn: dải sáng tăng dần rồi tắt.
 * - LED 7 thanh: đếm 0 -> 9, dấu chấm nhấp nháy.
 */
void LED_DEMO_APP(void)
{
    for (;;)
    {
        LED_DEMO_STEP();
        DELAY_MS(500);
    }
}

/*
 * LED_DEMO_STEP()
 * Một bước của hiệu ứng LED.
 */
void LED_DEMO_STEP(void)
{
    static unsigned char led_shift = 255;
    static unsigned char led_7_count = 0;

    /*
     * LED đơn.
     * 255 = 11111111: tắt hết.
     * Sau mỗi bước dịch trái để có thêm LED sáng.
     */
    PORTD = led_shift;

    if (led_shift != 0)
    {
        led_shift = led_shift << 1;
    }
    else
    {
        led_shift = 255;
    }

    /*
     * LED 7 thanh đếm.
     */
    LED7_OUT(led_7_count);

    /*
     * Đảo dấu chấm PC3.
     */
    PORTC ^= (1 << PC3);

    led_7_count = led_7_count + 1;

    if (led_7_count > 9)
    {
        led_7_count = 0;
    }
}

/*
 * PB_2_LED_APP()
 * Bài bấm nút theo tài liệu mục 3.3.
 *
 * Quy tắc:
 * - Phím 1: LED 7 thanh hiện 1, sáng 2 LED ngoài cùng bên trái
 * - Phím 2: LED 7 thanh hiện 2, sáng 2 LED tiếp theo
 * - Phím 3: LED 7 thanh hiện 3, sáng 2 LED tiếp theo
 * - Phím 4: LED 7 thanh hiện 4, sáng 2 LED ngoài cùng bên phải
 *
 * Hàm PB_CHECK() dùng return push_button, nên nhả nút vẫn giữ trạng thái cũ.
 */
void PB_2_LED_APP(void)
{
    for (;;)
    {
        push_button = PB_CHECK();

        LED7_OUT(push_button);

        switch (push_button)
        {
            case 1:
                PORTD = 0b11111100;
                break;

            case 2:
                PORTD = 0b11110011;
                break;

            case 3:
                PORTD = 0b11001111;
                break;

            case 4:
                PORTD = 0b00111111;
                break;

            default:
                PORTD = 0xFF;
                break;
        }

        /*
         * Delay nhỏ để chống rung phím.
         */
        DELAY_MS(30);
    }
}

/*
 * PB_CHECK()
 * Đọc trạng thái 4 nút trên kit.
 *
 * Khi bấm, chân tương ứng bị kéo xuống GND, tức mức logic 0.
 *
 * Bản này dùng:
 * return push_button;
 *
 * Nghĩa là khi không bấm nút nào, chương trình giữ lại trạng thái nút vừa bấm.
 */
unsigned char PB_CHECK(void)
{
    /*
     * Nếu 4 bit thấp của PINB khác 1111
     * nghĩa là có ít nhất một phím đang được bấm.
     */
    if ((PINB & 0x0F) != 0x0F)
    {
        if (!(PINB & (1 << PB0)))
        {
            return 1;
        }

        if (!(PINB & (1 << PB1)))
        {
            return 2;
        }

        if (!(PINB & (1 << PB2)))
        {
            return 3;
        }

        if (!(PINB & (1 << PB3)))
        {
            return 4;
        }
    }

    /*
     * Không bấm nút nào thì giữ trạng thái trước đó.
     * Đây chính là phần tài liệu gợi ý đổi từ return 0 thành return push_button.
     */
    return push_button;
}

/*
 * LED7_OUT()
 * Hiển thị số 0-9 trên LED 7 thanh.
 * Active-low: 0 sáng, 1 tắt.
 */
void LED7_OUT(unsigned char num)
{
    unsigned char temp = PORTC;

    /*
     * Giữ lại trạng thái dấu chấm PC3.
     */
    temp &= 0b00001000;

    switch (num)
    {
        case 0:
            temp |= 0b10000000;
            break;

        case 1:
            temp |= 0b11100011;
            break;

        case 2:
            temp |= 0b01000100;
            break;

        case 3:
            temp |= 0b01000001;
            break;

        case 4:
            temp |= 0b00100011;
            break;

        case 5:
            temp |= 0b00010001;
            break;

        case 6:
            temp |= 0b00010000;
            break;

        case 7:
            temp |= 0b11000011;
            break;

        case 8:
            temp |= 0b00000000;
            break;

        case 9:
            temp |= 0b00000001;
            break;

        default:
            temp |= 0b10000000;
            break;
    }

    PORTC = temp;
}

/*
 * DHT11_LCD_APP()
 * Đọc DHT11 rồi hiển thị nhiệt độ, độ ẩm lên LCD.
 * Tháo JP1, JP2.
 */
void DHT11_LCD_APP(void)
{
    unsigned char hum_i;
    unsigned char hum_d;
    unsigned char temp_i;
    unsigned char temp_d;
    unsigned char status;

    LCD4_INIT(0, 0);
    LCD4_OUT_CMD(0x01);

    LCD4_CUR_GOTO(1, 0);
    LCD4_OUT_STR("DHT11 + LCD");

    LCD4_CUR_GOTO(2, 0);
    LCD4_OUT_STR("Starting...");

    DELAY_MS(2000);

    for (;;)
    {
        status = DHT11_READ(&hum_i, &hum_d, &temp_i, &temp_d);

        if (status == DHT11_OK)
        {
            LCD_CLEAR_LINE(1);
            LCD4_CUR_GOTO(1, 0);
            LCD4_OUT_STR("Temp: ");
            LCD_PRINT_2DIGIT(temp_i);
            LCD4_OUT_DATA('.');
            LCD4_OUT_DEC(temp_d, 1);
            LCD4_OUT_DATA('C');

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

        DELAY_MS(2000);
    }
}

/*
 * DHT11_AND_LED_APP()
 * Đọc DHT11 và vẫn chạy LED đơn + LED 7 thanh.
 *
 * Lưu ý:
 * - Không dùng LCD trong chế độ này.
 * - DHT11 vẫn được đọc mỗi khoảng 2 giây.
 * - LED vẫn chạy theo hiệu ứng 0.5 giây/bước.
 */
void DHT11_AND_LED_APP(void)
{
    unsigned char hum_i;
    unsigned char hum_d;
    unsigned char temp_i;
    unsigned char temp_d;
    unsigned char status;
    unsigned char step_count = 0;

    DELAY_MS(2000);

    for (;;)
    {
        LED_DEMO_STEP();

        step_count++;

        /*
         * 4 bước x 500ms = khoảng 2 giây.
         * DHT11 không nên đọc quá nhanh.
         */
        if (step_count >= 4)
        {
            status = DHT11_READ(&hum_i, &hum_d, &temp_i, &temp_d);

            /*
             * Không dùng LCD nên chỉ đọc dữ liệu.
             * Các biến được ép void để tránh warning.
             */
            (void)status;
            (void)hum_i;
            (void)hum_d;
            (void)temp_i;
            (void)temp_d;

            step_count = 0;
        }

        DELAY_MS(500);
    }
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

/*
 * INIT_UART_USB_LCD()
 * Dùng cho bài giao tiếp máy tính qua UART-USB + LCD dịch chữ.
 * Tháo JP1, JP2 khi dùng LCD.
 */
void INIT_UART_USB_LCD(void)
{
    /*
     * UART:
     * PD0 = RXD input
     * PD1 = TXD output
     */
    DDRD &= ~(1 << PD0);
    DDRD |=  (1 << PD1);

    /*
     * LCD dùng:
     * RS -> PD6
     * RW -> PD5
     * E  -> PD7
     * D4 -> PC4
     * D5 -> PC5
     * D6 -> PC6
     * D7 -> PC7
     */
    DDRD |= (1 << PD5) | (1 << PD6) | (1 << PD7);
    PORTD &= ~((1 << PD5) | (1 << PD6) | (1 << PD7));

    DDRC |= 0xF0;
    PORTC |= 0x0F;

    /*
     * UART 9600 bps, 8 data bit, no parity, 1 stop bit.
     * F_CPU = 8 MHz => UBRR = 51.
     */
    UART_INIT(51, 8, 0, 1);
}

/*
 * LCD4_DIS_SHIFT()
 * Dịch toàn bộ màn hình LCD.
 *
 * lcd4_direct = 0: dịch phải
 * lcd4_direct = 1: dịch trái
 *
 * lcd4_step: số bước dịch
 */
void LCD4_DIS_SHIFT(unsigned char lcd4_direct, unsigned char lcd4_step)
{
    unsigned char i;

    if (lcd4_direct == 0)
    {
        /*
         * Dịch phải.
         */
        for (i = 0; i < lcd4_step; i++)
        {
            LCD4_OUT_CMD(0x1C);
        }
    }
    else
    {
        /*
         * Dịch trái.
         */
        for (i = 0; i < lcd4_step; i++)
        {
            LCD4_OUT_CMD(0x18);
        }
    }
}

/*
 * UART_USB_LCD_APP()
 * Gửi dữ liệu lên máy tính qua CP2102.
 * Đồng thời hiển thị và dịch chữ trên LCD1602.
 */
void UART_USB_LCD_APP(void)
{
    unsigned char shift_count = 0;

    LCD4_INIT(0, 0);
    LCD4_OUT_CMD(0x01);

    /*
     * Ghi chuỗi dài hơn 16 ký tự.
     * LCD chỉ nhìn thấy 16 ký tự đầu,
     * phần còn lại sẽ hiện ra khi dịch màn hình.
     */
    LCD4_CUR_GOTO(1, 0);
    LCD4_OUT_STR("Tran Ngoc Dung, Dien tu 09 - K68");

    LCD4_CUR_GOTO(2, 0);
    LCD4_OUT_STR("Vien DTVT, Truong DHBK HN");

    DELAY_MS(1000);

    for (;;)
    {
        /*
         * Gửi dữ liệu lên máy tính.
         */
        UART_TRAN_STR("Tran Ngoc Dung, Dien tu 09 - K68");
        UART_TRAN_BYTE(13);
        UART_TRAN_BYTE(10);

        UART_TRAN_STR("Vien DTVT, Truong DHBK HN");
        UART_TRAN_BYTE(13);
        UART_TRAN_BYTE(10);

        UART_TRAN_BYTE(13);
        UART_TRAN_BYTE(10);

        /*
         * Mỗi 1 giây dịch LCD sang trái 1 bước.
         */
        DELAY_MS(1000);
        LCD4_DIS_SHIFT(1, 1);

        shift_count++;

        /*
         * Sau một số lần dịch, reset lại màn hình để chữ quay về đầu.
         */
        if (shift_count >= 20)
        {
            shift_count = 0;

            LCD4_OUT_CMD(0x01);

            LCD4_CUR_GOTO(1, 0);
            LCD4_OUT_STR("Tran Ngoc Dung, Dien tu 09 - K68");

            LCD4_CUR_GOTO(2, 0);
            LCD4_OUT_STR("Vien DTVT, Truong DHBK HN");
        }
    }
}

#endif