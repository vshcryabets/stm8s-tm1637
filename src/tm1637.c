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

// Table which translates a digit into the segments
const unsigned char cDigit2Seg[] = {0x3f, 0x6, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f};

#ifdef USE_ARDUINO
#define usleep(X) delay(X)
#endif

/// <summary>
/// Initialize tm1637 with the clock and data pins
/// </summary>
void tm1637Init(struct TM1647State *state,
			   GPIO_TypeDef *bClock,
			   GPIO_Pin_TypeDef bClockP,
			   GPIO_TypeDef *bData,
			   GPIO_Pin_TypeDef bDataP)
{
	state->bClock = bClock;
	state->bClockP = bClockP;
	state->bData = bData;
	state->bDataP = bDataP;
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
	unsigned char i;

	for (i=0; i<8; i++)
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
	unsigned char bControl;
	if (b == 0) 
		// display off
		bControl = 0x80; 
	else
	{
		if (b > 8) b = 8;
		bControl = 0x88 | (b - 1);
	}
	tm1637Write(&bControl, 1, state);
} 

/// <summary>
/// Display a string of 4 digits and optional colon
/// by passing a string such as "12:34" or "45 67"
/// </summary>
void tm1637ShowDigits(char *pString, struct TM1647State* state)
{
	// commands and data to transmit
	unsigned char b, bTemp[16]; 
	unsigned char i, j;

	j = 0;
	// memory write command (auto increment mode)
	bTemp[0] = 0x40;
	tm1637Write(bTemp, 1, state);

	// set display address to first digit command
	bTemp[j++] = 0xc0;
	for (i=0; i<5; i++)
	{
   	// position of the colon
		if (i == 2) 
		{
	  	// turn on correct bit
			if (pString[i] == ':')
			{
		 	// second digit high bit controls colon LEDs
				bTemp[2] |= 0x80;
			}
		}
		else
		{
			b = 0;
			if (pString[i] >= '0' && pString[i] <= '9')
			{
			// segment data
				b = cDigit2Seg[pString[i] & 0xf]; 
			}
			bTemp[j++] = b;
		}
	}
	// send to the display
	tm1637Write(bTemp, j, state); 
}