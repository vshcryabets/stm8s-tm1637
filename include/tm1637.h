// indigo6alpha's TM1637 library for STM8SF103F3 MCU
// Project started 3/17/2018
// written by indigo6alpha (indigosixalpha164@gmail.com)
//            2024 vshkriabets@2vsoft.com
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>

#ifdef USE_ARDUINO
    #include "Arduino.h"
#elif USE_STM8
    #include "stm8s.h"
    #include "stm8s_gpio.h"
#endif

#ifdef WITH_DEC_FONT
extern char tm1637DecFont[12];
#endif
#ifdef WITH_HEX_FONT
extern char tm1637HexFont[18];
#endif

struct TM1637State
{
    GPIO_TypeDef *bClock;
    GPIO_Pin_TypeDef bClockP;
    GPIO_TypeDef *bData;
    GPIO_Pin_TypeDef bDataP;
    const uint8_t *font;
    uint8_t ledCount;
#ifndef USE_6SEG    
    uint8_t buffer[5];
#else
    uint8_t buffer[7];
#endif    
};

struct TM1637Config {
    GPIO_TypeDef *clockPort;
    GPIO_Pin_TypeDef clockPin;
    GPIO_TypeDef *dataPort;
    GPIO_Pin_TypeDef dataPin;
    const uint8_t *font;
};

/// <summary>
/// Initialize tm1637 with the clock and data pins
/// </summary>
void tm1637Init(struct TM1637State *state,
                struct TM1637Config *config);

/// <summary>
/// Start wire transaction
/// </summary>
static void tm1637Start(struct TM1637State*);

/// <summary>
/// Stop wire transaction
/// </summary>
static void tm1637Stop(struct TM1637State*);

/// <summary>
/// Get data acknowledgement
/// </summary>
static unsigned char tm1637GetAck(struct TM1637State*);

/// <summary>
/// Write a sequence of unsigned chars to the controller
/// </summary>
static void tm1637Write(unsigned char *pData, unsigned char bLen, struct TM1637State*);

/// <summary>
/// Set brightness (0-8)
/// </summary>
void tm1637SetBrightness(unsigned char b, struct TM1637State*);

/// <summary>
/// Display a string of 4 digits and optional colon
/// by passing a string such as "12:34" or "45 67"
/// </summary>
void tm1637ShowDigits(char *pString, struct TM1637State*);
/// <summary>
/// Display a 4 digits decimal without colon
/// </summary>
void tm1637ShowInt(struct TM1637State *state, uint16_t value, bool showColon);
/// <summary>
/// Show or hide colon.
/// </summary>
void tm1637ShowColon(struct TM1637State* state, bool value);