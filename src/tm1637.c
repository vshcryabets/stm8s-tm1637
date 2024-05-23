// indigo6alpha's TM1637 library for STM8SF103F3 MCU
// Project started 6/3/2018
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

#include "tm1637.h"

#define CLOCK_DELAY 0

#ifdef WITH_DEC_FONT
char tm1637DecFont[12] = {0x30, // base char
	 10, // symbols count
	 0x3f, // 0
	 0x06,  // 1
	 0x5b, // 2
	 0x4f, // 3
	 0x66, // 4
	 0x6d, // 5
	 0x7d, // 6
	 0x07, // 7
	 0x7f, // 8
	 0x6f // 9,
	 };
#endif

#ifdef WITH_HEX_FONT
char tm1637HexFont[18] = {0x30, // base char
	 16, // symbols count
	 0x3f, // 0
	 0x06,  // 1
	 0x5b, // 2
	 0x4f, // 3
	 0x66, // 4
	 0x6d, // 5
	 0x7d, // 6
	 0x07, // 7
	 0x7f, // 8
	 0x77, // 9,
	 0x77, // A
	 0x7C, // b
	 0x39, //C
	 0x5e, //d
	 0x79, //E
	 0x71 // F
	 };
#endif

#ifdef USE_ARDUINO
#define usleep(X) delay(X)
#endif

const uint8_t TM1637_CMD_WRITE_MEMORY = 0x40;
const uint8_t TM1637_CMD_SET_ADDR = 0xC0;

/// <summary>
/// Initialize tm1637 with the clock and data pins
/// </summary>
void tm1637Init(struct TM1647State *state,
			   GPIO_TypeDef *bClock,
			   GPIO_Pin_TypeDef bClockP,
			   GPIO_TypeDef *bData,
			   GPIO_Pin_TypeDef bDataP,
               const uint8_t *font)
{
	state->bClock = bClock;
	state->bClockP = bClockP;
	state->bData = bData;
	state->bDataP = bDataP;
	state->font = font;
	state->ledCount = 4;
	GPIO_Init(bClock, bClockP, GPIO_MODE_OUT_PP_LOW_FAST); 
	GPIO_Init(bData, bDataP, GPIO_MODE_OUT_PP_LOW_FAST);
	GPIO_WriteLow(bClock, bClockP);
	GPIO_WriteLow(bData, bDataP);
}

/// <summary>
/// Start wire transaction
/// </summary>
static void tm1637Start(struct TM1647State* state)
{
	GPIO_WriteHigh(state->bData, state->bDataP);
	GPIO_WriteHigh(state->bClock, state->bClockP);
	usleep(CLOCK_DELAY);
	GPIO_WriteLow(state->bData, state->bDataP);
}

/// <summary>
/// Stop wire transaction
/// </summary>
static void tm1637Stop(struct TM1647State* state)
{
	// clock low
	GPIO_WriteLow(state->bClock, state->bClockP);
	usleep(CLOCK_DELAY);
	// data low
	GPIO_WriteLow(state->bData, state->bDataP);
	usleep(CLOCK_DELAY);
	// clock high
	GPIO_WriteHigh(state->bClock, state->bClockP);
	usleep(CLOCK_DELAY);
	// data high
	GPIO_WriteHigh(state->bData, state->bDataP);
} 

/// <summary>
/// Get data ack
/// </summary>
static unsigned char tm1637GetAck(struct TM1647State* state)
{
	unsigned char bAck = 1;

	// read ack
	// clock to low
	GPIO_WriteLow(state->bClock, state->bClockP);
	// data as input
	
	usleep(CLOCK_DELAY);

	// clock high
	GPIO_WriteHigh(state->bClock, state->bClockP);
	usleep(CLOCK_DELAY);
	// clock to low
	GPIO_WriteLow(state->bClock, state->bClockP);
	return bAck;
}


/// <summary>
/// Write a unsigned char to the controller
/// </summary>
static void tm1637WriteByte(unsigned char b, struct TM1647State* state)
{
	for (uint8_t i=0; i<8; i++)
	{
		// clock low
		GPIO_WriteLow(state->bClock, state->bClockP);
		// LSB to MSB
		if (b & 1) 
			GPIO_WriteHigh(state->bData, state->bDataP);
		else
			GPIO_WriteLow(state->bData, state->bDataP);
		usleep(CLOCK_DELAY);
		// clock high
		GPIO_WriteHigh(state->bClock, state->bClockP);
		usleep(CLOCK_DELAY);
		b >>= 1;
	}
}

/// <summary>
/// Write a sequence of unsigned chars to the controller
/// </summary>
static void tm1637Write(unsigned char *pData, unsigned char bLen, struct TM1647State* state)
{
	unsigned char b, bAck;
	bAck = 1;
	tm1637Start(state);
	for (b=0; b<bLen; b++)
	{
		tm1637WriteByte(pData[b], state);
		bAck &= tm1637GetAck(state);
	}
	tm1637Stop(state);
} 

/// <summary>
/// Set brightness (0-8)
/// </summary>
void tm1637SetBrightness(unsigned char b, struct TM1647State* state)
{
	uint8_t bControl = 0x80;
	if (b > 0) 
	{
		if (b > 8) 
			b = 8;
		bControl = 0x88 | (b - 1);
	}
	tm1637Write(&bControl, 1, state);
} 

/// <summary>
/// Display a string of 4 digits and optional colon
/// by passing a string such as "12:34" or "45 67"
/// </summary>
// void tm1637ShowDigits(char *pString, struct TM1647State* state)
// {
// 	// memory write command (auto increment mode)
// 	tm1637Write(&TM1637_CMD_WRITE_MEMORY, 1, state);

// 	// set display address to first digit command
// 	// commands and data to transmit
// 	unsigned char bTemp[16]; 
// 	uint8_t i;
// 	uint8_t j = 0;
// 	bTemp[j++] = 0xc0;
// 	for (i=0; i<5; i++)
// 	{
//    	// position of the colon
// 		if (i == 2) 
// 		{
// 	  		// turn on correct bit
// 			if (pString[i] == ':')
// 			{
// 		 	// second digit high bit controls colon LEDs
// 				bTemp[2] |= 0x80;
// 			}
// 		}
// 		else
// 		{
// 			uint8_t b = 0;
// 			if (pString[i] >= state->font[0]) {
// 				uint8_t idx = pString[i] - state->font[0];
// 				if (idx < state->font[1]) {
// 					// segment data
// 					b = state->font[2 + idx]; 
// 				}
// 			}
// 			bTemp[j++] = b;
// 		}
// 	}
// 	// send to the display
// 	tm1637Write(bTemp, j, state); 
// }

void tm1637ShowDigits(char *pString, struct TM1647State* state)
{
	unsigned char bTemp[6]; 
	uint8_t i;
	uint8_t j = 0;
	for (i=0; i<5; i++)
	{
	   	// position of the colon
		if (i == 2) 
		{
	  		// turn on correct bit
			if (pString[i] == ':')
			{
		 	// second digit high bit controls colon LEDs
				bTemp[1] |= 0x80;
			}
		}
		else
		{
			uint8_t b = 0;
			if (pString[i] >= state->font[0]) {
				uint8_t idx = pString[i] - state->font[0];
				if (idx < state->font[1]) {
					// segment data
					b = state->font[2 + idx]; 
				}
			}
			bTemp[j++] = b;
		}
	}
	// send to the display
	tm1637WriteFontBytes(state, bTemp, j);
}

// uint8_t tm1637charToFont(struct TM1647State* state, char c) {
// 	uint8_t b = 0;
// 	if (c >= state->font[0]) {
// 		uint8_t idx = c - state->font[0];
// 		if (idx < state->font[1]) {
// 			// segment data
// 			b = state->font[2 + idx]; 
// 		}
// 	}
// 	return b;
// }

// uint8_t tm1637decToFont(uint8_t dec);

void tm1637WriteFontBytes(struct TM1647State* state, char *symbols, uint8_t len)
{
	// memory write command (auto increment mode)
	tm1637Write(&TM1637_CMD_WRITE_MEMORY, 1, state);
	tm1637Write(&TM1637_CMD_SET_ADDR, 1, state);
	tm1637Write(symbols, len, state);
}

void tm1637ShowInt(struct TM1647State* state, uint16_t value) {
	uint8_t buffer[6];
	for (uint8_t i = 0; i < state->ledCount; i++) {
		uint8_t lowVal = value % 10;
		buffer[3 - i] = state->font[2 + lowVal];
		value = value / 10;
	}
}