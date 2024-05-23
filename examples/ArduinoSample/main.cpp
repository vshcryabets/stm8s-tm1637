#include "Arduino.h"
#include "tm1637.h"

struct TM1647State uiState;

void setup() {
    tm1637Init(&uiState, GPIOA, GPIO_PIN_1, GPIOA, GPIO_PIN_2); 
    tm1637SetBrightness(3, &uiState);
    char *digits = "12 34";
    tm1637ShowDigits(szTemp, &uiState);
}

void loop() {
}
