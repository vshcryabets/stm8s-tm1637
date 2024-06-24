#include "Arduino.h"
#define USE_ARDUINO 
#define WITH_DEC_FONT
#include "tm1637.h"

struct TM1637State tm1637;
uint16_t counter;

void setup()
{
    tm1637Init(&tm1637,
               GPIOA, GPIO_PIN_1,
               GPIOA, GPIO_PIN_2,
               tm1637DecFont);
    tm1637SetBrightness(3, &tm1637);
    char *digits = "12 34";
    tm1637ShowDigits(digits, &tm1637);
    counter = 1;
}

void loop()
{
    tm1637ShowInt(&tm1637, counter);
    counter++;
}
