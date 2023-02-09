#include <Arduino.h>

#define D4 5
#define D5 4
#define D6 3
#define D7 2
#define RS 7
#define EN 6
#define TRIG 10
#define ECHO 11

uint8_t lcd_numbers[10]{
        0x30, // 0
        0x31, // 1
        0x32, // 2
        0x33, // 3
        0x34, // 4
        0x35, // 5
        0x36, // 6
        0x37, // 7
        0x38, // 8
        0x39, // 9
};

uint8_t lcd_distance[10]{
        0xC4, // Д
        0xE8, // и
        0xF1, // с
        0xF2, // т
        0xE0, // а
        0xED, // н
        0xF6, // ц
        0xE8, // и
        0xFF, // я
        0x3A, // :
};


void send4(uint8_t value) {
    // отправляем наши 4 бита на дисплей
    digitalWrite(D4, (value >> 0) & 0x01);
    digitalWrite(D5, (value >> 1) & 0x01);
    digitalWrite(D6, (value >> 2) & 0x01);
    digitalWrite(D7, (value >> 3) & 0x01);

    // бип
    digitalWrite(EN, LOW);
    delayMicroseconds(1);
    digitalWrite(EN, HIGH);
    delayMicroseconds(1);
    digitalWrite(EN, LOW);
    delayMicroseconds(300);
}

void send_command(uint8_t value) { // отправляем команду на дисплей
    digitalWrite(RS, LOW);
    send4(value >> 4);
    send4(value);
}

void write(uint8_t value) { // пишем на дисплей
    digitalWrite(RS, HIGH);
    send4(value >> 4);
    send4(value);
}

int char_to_int(char s) { // преобразование символа в цифру
    switch (s) {
        case '0':
            return 0;
        case '1':
            return 1;
        case '2':
            return 2;
        case '3':
            return 3;
        case '4':
            return 4;
        case '5':
            return 5;
        case '6':
            return 6;
        case '7':
            return 7;
        case '8':
            return 8;
        case '9':
            return 9;
    }
    return 0; // если что-то пойдет не так
}

void lcd_hide_cursor() {
    send_command(0x80 | 0x12); // прячем курсор
}

void lcd_print_distance(int x) {
    String xstr = String(x); // число в строку
    byte lenx = xstr.length(); // смотрим сколько символов

    switch (lenx) {
        case 1:
            write(lcd_numbers[char_to_int(xstr[0])]); // число
            write(0x20); // пропуск
            write(0x20); // пропуск
            write(0x20); // пропуск
            break;
        case 2:
            write(lcd_numbers[char_to_int(xstr[0])]); // число
            write(lcd_numbers[char_to_int(xstr[1])]); // число
            write(0x20); // пропуск
            write(0x20); // пропуск
            break;
        case 3:
            write(lcd_numbers[char_to_int(xstr[0])]); // число
            write(lcd_numbers[char_to_int(xstr[1])]); // число
            write(lcd_numbers[char_to_int(xstr[2])]); // число
            write(0x20); // пропуск
            break;
        case 4:
            write(lcd_numbers[char_to_int(xstr[0])]); // число
            write(lcd_numbers[char_to_int(xstr[1])]); // число
            write(lcd_numbers[char_to_int(xstr[2])]); // число
            write(lcd_numbers[char_to_int(xstr[3])]); // число
            break;
    }
    write(0x20); // *пустота*
    write(0xEC); // м
    write(0xEC); // м

    lcd_hide_cursor();
}

int measure_distance() {
    uint32_t duration, distance;
    digitalWrite(TRIG, LOW);  // выключаем
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH); // Теперь установим высокий уровень на пине Trig
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    duration = pulseIn(ECHO, HIGH); // Узнаем длительность высокого сигнала на пине Echo

    if (duration == 0) {
        return 0; // если сигнал не получен возвращаем ошибку
    }

    distance = duration / 8.66;  // Рассчитаем расстояние. Коэффициент подобран

    if (distance < 20) { // если меньше, то ошибка
        return 0;
    } else if (distance > 3500) { // если больше, ошибка
        return 0;
    } else {
        return distance;  // все устраивает
    }
}

void lcd_unit() {
    pinMode(D4, OUTPUT); // настраимваем пины
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    pinMode(RS, OUTPUT);
    pinMode(EN, OUTPUT);
    delayMicroseconds(50000);

    digitalWrite(RS, LOW);
    digitalWrite(EN, LOW);
    delayMicroseconds(150);
    send4(0x03);
    delayMicroseconds(4500); // wait 4.5ms
    send4(0x03);
    delayMicroseconds(4500); // wait 4.5ms
    send4(0x03);
    send_command(0x40); // LCD_FUNCTIONSET
    send_command(0x12); // LCD_DISPLAYCONTROL
    send_command(0x01); // LCD_CLEAR
    delayMicroseconds(2000);
    send_command(0x06); // LCD_ENTRYMODESET
    delayMicroseconds(2000);

}

void setup() {
    pinMode(TRIG, OUTPUT);
    lcd_unit();
    delayMicroseconds(150);
    for (byte i; i <= 9; i++) {
        write(lcd_distance[i]);
        delayMicroseconds(100);
    }
}

void loop() {
    delay(700);
    send_command(0x80 | 9 + 0x40); // ставим курсор на 9 колонку 2 строки
    int dist = measure_distance();
    if (measure_distance() != 0) { // если не ошибка
        lcd_print_distance(dist);
    } else {
        write(0x45); // пишем Err
        write(0x72);
        write(0x72);
    }
}