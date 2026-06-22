#include <avr/io.h>

#define FRE 8

/*
 * Chọn 1 trong các chế độ:
 *
 * APP_LED_DEMO:
 *   LED đơn sáng dần từng con, LED 7 thanh đếm 0->9.
 *   Cắm JP1, JP2. Không dùng LCD.
 *
 * APP_BUTTON_LED:
 *   Bấm nút PB1-PB4, LED 7 thanh hiện số 1-4,
 *   dãy LED đơn sáng theo từng cặp.
 *   Có return push_button: nhả nút vẫn giữ trạng thái vừa bấm.
 *   Cắm JP1, JP2. Không dùng LCD.
 *
 * APP_DHT_LCD:
 *   DHT11 hiển thị nhiệt độ/độ ẩm lên LCD.
 *   Tháo JP1, JP2. Dùng LCD.
 *
 * APP_DHT_AND_LED_NO_LCD:
 *   DHT11 vẫn đọc, LED đơn + LED 7 thanh vẫn chạy.
 *   Cắm JP1, JP2. Không dùng LCD.
 */

#define APP_LED_DEMO              1
#define APP_BUTTON_LED            2
#define APP_DHT_LCD               3
#define APP_DHT_AND_LED_NO_LCD    4

#define APP_MODE APP_DHT_LCD

/*
 * Biến toàn cục lưu phím đang/đã được nhấn.
 * Dùng cho bài APP_BUTTON_LED.
 */
unsigned char push_button = 0;

#include "hunget_lcd.h"
#include "dht11.h"
#include "thu_vien_rieng.h"

int main(void)
{
#if APP_MODE == APP_LED_DEMO

    INIT_LED_DEMO();
    LED_DEMO_APP();

#elif APP_MODE == APP_BUTTON_LED

    INIT_BUTTON_LED();
    PB_2_LED_APP();

#elif APP_MODE == APP_DHT_LCD

    INIT_DHT_LCD();
    DHT11_LCD_APP();

#elif APP_MODE == APP_DHT_AND_LED_NO_LCD

    INIT_DHT_LED_NO_LCD();
    DHT11_AND_LED_APP();

#else

    INIT_BUTTON_LED();
    PB_2_LED_APP();

#endif

    return 0;
}