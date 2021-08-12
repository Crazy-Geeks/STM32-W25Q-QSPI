/**
 *******************************************
 * @file    libs.h
 * @author  Dmitriy Semenov / Crazy_Geeks
 * @brief   Internal header for adding sys libs and defines
 *******************************************
*/

#ifndef LIBS_H_
#define LIBS_H_

#include "main.h"	 ///< Main project file
#include <stdint.h>  ///< Std types
#include <stdbool.h> ///< _Bool to bool
#include <string.h>	 ///< Lib for sprintf, strlen, etc

typedef uint8_t u8_t; 	///< 8-bit unsigned
typedef int8_t i8_t;	///< 8-bit signed
typedef uint16_t u16_t; ///< 16-bit unsigned
typedef int16_t i16_t;	///< 16-bit signed
typedef uint32_t u32_t; ///< 32-bit unsigned
typedef int32_t i32_t;	///< 32-bit signed
typedef float_t fl_t;	///< float type

#define delay(x) HAL_Delay(x) ///< arduino-supportable delay or RTOS support ability

#endif /* LIBS_H_ */
