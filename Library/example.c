/*
 * example.c
 *
 *  Created on: Aug 13, 2021
 *      Author: Semenov Dmitriy
 *      URL: https://github.com/Crazy-Geeks/STM32-W25Q-QSPI
 */

#include "w25q_mem.h"

void main(void) {
	W25Q_Init();		 // init the chip
	W25Q_EraseSector(0); // erase 4K sector - required before recording

	// make test data
	u8_t byte = 0x65;
	u8_t byte_read = 0;
	u8_t in_page_shift = 0;
	u8_t page_number = 0;
	// write data
	W25Q_ProgramByte(byte, in_page_shift, page_number);
	// read data
	W25Q_ReadByte(&byte_read, in_page_shift, page_number);

	// make example structure
	struct STR {
		u8_t abc;
		u32_t bca;
		char str[4];
		fl_t gg;
	} _str, _str2;

	// fill instance
	_str.abc = 0x20;
	_str.bca = 0x3F3F4A;
	_str.str[0] = 'a';
	_str.str[1] = 'b';
	_str.str[2] = 'c';
	_str.str[3] = '\0';
	_str.gg = 0.658;

	u16_t len = sizeof(_str);	// length of structure in bytes

	// program structure
	W25Q_ProgramData((u8_t*) &_str, len, ++in_page_shift, page_number);
	// read structure to another instance
	W25Q_ReadData((u8_t*) &_str2, len, in_page_shift, page_number);

	W25Q_Sleep();	// go to sleep

	__NOP();	// place for breakpoint

	while (1)
		;

}
