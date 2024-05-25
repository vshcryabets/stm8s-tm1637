# TM1637 C library for STM8 MCUs

A library to configure and control TM1637 4-digit 7-segment display module. This library also contains the GPIO and CLK headers for STM8 MCUs.

## Build flags
**USE_ARDUINO** - build with arduino framework.

**USE_STM8** - use STM* headers.
**USE_6SEG** - reserve buffers for support 6 digits LED display. Otherwise buffers will be reserved only for 4 digits.

**WITH_DEC_FONT** - include font for 0-9 numbers (tm1637DecFont array).
**WITH_HEX_FONT** - include font for 0-9A-F numbers (tm1637HexFont array).