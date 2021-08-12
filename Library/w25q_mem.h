/**
 *******************************************
 * @file    w25q_mem.h
 * @author  Dmitriy Semenov / Crazy_Geeks
 * @version 0.1b
 * @date	12-August-2021
 * @brief   Header for W25Qxxx lib
 * @note 	https://github.com/Crazy-Geeks/STM32-W25Q-QSPI
 *******************************************
 *
 * @note https://ru.mouser.com/datasheet/2/949/w25q256jv_spi_revg_08032017-1489574.pdf
 * @note https://www.st.com/resource/en/application_note/DM00227538-.pdf
*/

#ifndef W25Q_QSPI_W25Q_MEM_H_
#define W25Q_QSPI_W25Q_MEM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "libs.h"

/**
 * @addtogroup W25Q_Driver
 * @brief W25Q QSPI Driver
 * @{
 */

/**
 * @defgroup W25Q_Param W25Q Chip's Parameters
 * @brief User's chip parameters
 * @{
 */
// YOUR CHIP'S SETTINGS
/// Mem size in MB-bit
#define MEM_FLASH_SIZE 256U 	// 256 MB-bit
/// Mem big block size in KB
#define MEM_BLOCK_SIZE 64U		// 64 KB: 256 pages
/// Mem small block size in KB
#define MEM_SBLOCK_SIZE 32U		// 32 KB: 128 pages
/// Mem sector size in KB
#define MEM_SECTOR_SIZE 4U		// 4 KB : 16 pages
/// Mem page size in bytes
#define MEM_PAGE_SIZE  256U		// 256 byte : 1 page
/// Blocks count
#define BLOCK_COUNT (MEM_FLASH_SIZE * 2) // 512 blocks
/// Sector count
#define SECTOR_COUNT (BLOCK_COUNT * 16)  // 8192 sectors
/// Pages count
#define PAGE_COUNT (SECTOR_COUNT * 16)	 // 131'072 pages

/**@}*/

/**
 * @enum W25Q_STATE
 * @brief W25Q Return State
 * Lib's functions status returns
 * @{
 */
typedef enum{
	W25Q_OK = 0,  		///< Chip OK - Execution fine
	W25Q_BUSY = 1,		///< Chip busy
	W25Q_PARAM_ERR = 2, ///< Function parameters error
	W25Q_CHIP_ERR = 3,	///< Chip error
	W25Q_SPI_ERR = 4, 	///< SPI Bus err
	W25Q_CHIP_IGNORE = 5, ///< Chip ignore state
}W25Q_STATE;
/** @} */

/**
 * @struct W25Q_STATUS_REG
 * @brief  W25Q Status Registers
 * @TODO: Mem protected recognition
 *
 * Structure to check chip's status registers
 * @{
 */
typedef struct{
	bool BUSY;  ///< Erase/Write in progress
	bool WEL;	///< Write enable latch (1 - write allowed)
	bool QE;	///< Quad SPI mode
	bool SUS; 	///< Suspend Status
	bool ADS; 	///< Current addr mode (0-3 byte / 1-4 byte)
	bool ADP; 	///< Power-up addr mode
	bool SLEEP; ///< Sleep Status
}W25Q_STATUS_REG;
/** @} */


W25Q_STATE W25Q_Init(void);		///< Initalize function

W25Q_STATE W25Q_EnableVolatileSR(void);						 ///< Make Status Register Volatile
W25Q_STATE W25Q_ReadStatusReg(u8_t *reg_data, u8_t reg_num); ///< Read status register to variable
W25Q_STATE W25Q_WriteStatusReg(u8_t reg_data, u8_t reg_num);///< Write status register from variable
W25Q_STATE W25Q_ReadStatusStruct(W25Q_STATUS_REG *status);	 ///< Read all status registers to struct
W25Q_STATE W25Q_IsBusy(void);	///< Check chip's busy status

W25Q_STATE W25Q_ReadSByte(i8_t *buf, u8_t pageShift, u32_t pageNum);			///< Read signed 8-bit variable
W25Q_STATE W25Q_ReadByte(u8_t *buf, u8_t pageShift, u32_t pageNum);			 	///< Read 8-bit variable
W25Q_STATE W25Q_ReadSWord(i16_t *buf, u8_t pageShift, u32_t pageNum);			///< Read signed 16-bit variable
W25Q_STATE W25Q_ReadWord(u16_t *buf, u8_t pageShift, u32_t pageNum);			///< Read 16-bit variable
W25Q_STATE W25Q_ReadSLong(i32_t *buf, u8_t pageShift, u32_t pageNum);			///< Read signed 32-bit variable
W25Q_STATE W25Q_ReadLong(u32_t *buf, u8_t pageShift, u32_t pageNum);			///< Read 32-bit variable
W25Q_STATE W25Q_ReadData(u8_t *buf, u16_t len, u8_t pageShift, u32_t pageNum);  ///< Read any 8-bit data
W25Q_STATE W25Q_ReadRaw(u8_t *buf, u16_t data_len, u32_t rawAddr);				///< Read data from raw addr
W25Q_STATE W25Q_SingleRead(u8_t *buf, u32_t len, u32_t Addr);					///< Read data from raw addr by single line

W25Q_STATE W25Q_EraseSector(u32_t SectAddr);			///< Erase 4KB Sector
W25Q_STATE W25Q_EraseBlock(u32_t BlockAddr, u8_t size); ///< Erase 32KB/64KB Sector
W25Q_STATE W25Q_EraseChip(void);						///< Erase all chip

W25Q_STATE W25Q_ProgramSByte(i8_t buf, u8_t pageShift, u32_t pageNum);			 ///< Program signed 8-bit variable
W25Q_STATE W25Q_ProgramByte(u8_t buf, u8_t pageShift, u32_t pageNum);			 ///< Program 8-bit variable
W25Q_STATE W25Q_ProgramSWord(i16_t buf, u8_t pageShift, u32_t pageNum);			 ///< Program signed 16-bit variable
W25Q_STATE W25Q_ProgramWord(u16_t buf, u8_t pageShift, u32_t pageNum);			 ///< Program 16-bit variable
W25Q_STATE W25Q_ProgramSLong(i32_t buf, u8_t pageShift, u32_t pageNum);			 ///< Program signed 32-bit variable
W25Q_STATE W25Q_ProgramLong(u32_t buf, u8_t pageShift, u32_t pageNum);			 ///< Program 32-bit variable
W25Q_STATE W25Q_ProgramData(u8_t *buf, u16_t len, u8_t pageShift, u32_t pageNum); ///< Program any 8-bit data
W25Q_STATE W25Q_ProgramRaw(u8_t *buf, u16_t data_len, u32_t rawAddr); 					 ///< Program data to raw addr

W25Q_STATE W25Q_SetBurstWrap(u8_t WrapSize);		///< Set Burst with Wrap

W25Q_STATE W25Q_ProgSuspend(void);	///< Pause Programm/Erase operation
W25Q_STATE W25Q_ProgResume(void);	///< Resume Programm/Erase operation

W25Q_STATE W25Q_Sleep(void);	///< Set low current consumption
W25Q_STATE W25Q_WakeUP(void);	///< Wake the chip up from sleep mode

W25Q_STATE W25Q_ReadID(u8_t *buf);				///< Read chip ID
W25Q_STATE W25Q_ReadFullID(u8_t *buf);			///< Read full chip ID (Manufacturer ID + Device ID)
W25Q_STATE W25Q_ReadUID(u8_t *buf);				///< Read unique chip ID
W25Q_STATE W25Q_ReadJEDECID(u8_t *buf); 		///< Read ID by JEDEC Standards
W25Q_STATE W25Q_ReadSFDPRegister(u8_t *buf); 	///< Read device descriptor (SFDP Standard)

W25Q_STATE W25Q_EraseSecurityRegisters(u8_t numReg);							///< Erase security register
W25Q_STATE W25Q_ProgSecurityRegisters(u8_t *buf, u8_t numReg, u8_t byteAddr);	///< Program security register
W25Q_STATE W25Q_ReadSecurityRegisters(u8_t *buf, u8_t numReg, u8_t byteAddr);	///< Read security register

W25Q_STATE W25Q_BlockReadOnly(u32_t Addr, bool enable);			///< Individual block/sector read-only lock
W25Q_STATE W25Q_BlockReadOnlyCheck(bool *state, u32_t Addr);	///< Check block's/sector's read-only lock status
W25Q_STATE W25Q_GlobalReadOnly(bool enable);		///< Set read-only param to all chip

W25Q_STATE W25Q_SwReset(bool force);	///< Software reset


/**
 * @defgroup W25Q_Commands W25Q Chip's Commands
 * @brief W25Q Chip commands from datasheet
 * @{
 */
#define W25Q_WRITE_ENABLE 0x06U			///< sets WEL bit, must be set before any write/program/erase
#define W25Q_WRITE_DISABLE 0x04U		///< resets WEL bit (state after power-up)
#define W25Q_ENABLE_VOLATILE_SR 0x50U	///< check 7.1 in datasheet
#define W25Q_READ_SR1 0x05U				///< read status-register 1
#define W25Q_READ_SR2 0x35U				///< read status-register 2
#define W25Q_READ_SR3 0x15U				///< read ststus-register 3
#define W25Q_WRITE_SR1 0x01U			///< write status-register 1 (8.2.5)
#define W25Q_WRITE_SR2 0x31U			///< write status-register 2 (8.2.5)
#define W25Q_WRITE_SR3 0x11U			///< write status-register 3 (8.2.5)
#define W25Q_READ_EXT_ADDR_REG 0xC8U	///< read extended addr reg (only in 3-byte mode)
#define W25Q_WRITE_EXT_ADDR_REG 0xC8U	///< write extended addr reg (only in 3-byte mode)
#define W25Q_ENABLE_4B_MODE 0xB7U			///< enable 4-byte mode (128+ MB address)
#define W25Q_DISABLE_4B_MODE 0xE9U			///< disable 4-byte mode (<=128MB)
#define W25Q_READ_DATA 0x03U				///< read data by standard SPI
#define W25Q_READ_DATA_4B 0x13U				///< read data by standard SPI in 4-byte mode
#define W25Q_FAST_READ 0x0BU				///< highest FR speed (8.2.12)
#define W25Q_FAST_READ_4B 0x0CU				///< fast read in 4-byte mode
#define W25Q_FAST_READ_DUAL_OUT 0x3BU		///< fast read in dual-SPI OUTPUT (8.2.14)
#define W25Q_FAST_READ_DUAL_OUT_4B 0x3CU	///< fast read in dual-SPI OUTPUT in 4-byte mode
#define W25Q_FAST_READ_QUAD_OUT 0x6BU		///< fast read in quad-SPI OUTPUT (8.2.16)
#define W25Q_FAST_READ_QUAD_OUT_4B 0x6CU	///< fast read in quad-SPI OUTPUT in 4-byte mode
#define W25Q_FAST_READ_DUAL_IO 0xBBU		///< fast read in dual-SPI I/O (address transmits by both lines)
#define W25Q_FAST_READ_DUAL_IO_4B 0xBCU		///< fast read in dual-SPI I/O in 4-byte mode
#define W25Q_FAST_READ_QUAD_IO 0xEBU		///< fast read in quad-SPI I/O (address transmits by quad lines)
#define W25Q_FAST_READ_QUAD_IO_4B 0xECU		///< fast read in quad-SPI I/O in 4-byte mode
#define W25Q_SET_BURST_WRAP 0x77U			///< use with quad-I/O (8.2.22)
#define W25Q_PAGE_PROGRAM 0x02U				///< program page (256bytes) by single SPI line
#define W25Q_PAGE_PROGRAM_4B 0x12U			///< program page by single SPI in 4-byte mode
#define W25Q_PAGE_PROGRAM_QUAD_INP 0x32U	///< program page (256bytes) by quad SPI lines
#define W25Q_PAGE_PROGRAM_QUAD_INP_4B 0x34U ///< program page by quad SPI in 4-byte mode
#define W25Q_SECTOR_ERASE 0x20U				///< sets all 4Kbyte sector with 0xFF (erases it)
#define W25Q_SECTOR_ERASE_4B 0x21U			///< sets all 4Kbyte sector with 0xFF in 4-byte mode
#define W25Q_32KB_BLOCK_ERASE 0x52U			///< sets all 32Kbyte block with 0xFF
#define W25Q_64KB_BLOCK_ERASE 0xD8U			///< sets all 64Kbyte block with 0xFF
#define W25Q_64KB_BLOCK_ERASE_4B 0xDCU		///< sets all 64Kbyte sector with 0xFF in 4-byte mode
#define W25Q_CHIP_ERASE 0xC7U				///< fill all the chip with 0xFF
//#define W25Q_CHIP_ERASE 0x60U				///< another way to erase chip
#define W25Q_ERASEPROG_SUSPEND 0x75U		///< suspend erase/program operation (can be applied only when SUS=0, BYSY=1)
#define W25Q_ERASEPROG_RESUME 0x7AU			///< resume erase/program operation (if SUS=1, BUSY=0)
#define W25Q_POWERDOWN 0xB9U				///< powers down the chip (power-up by reading ID)
#define W25Q_POWERUP 0xABU					///< release power-down
#define W25Q_DEVID 0xABU					///< read Device ID (same as powerup)
#define W25Q_FULLID 0x90U					///< read Manufacturer ID & Device ID
#define W25Q_FULLID_DUAL_IO 0x92U			///< read Manufacturer ID & Device ID by dual I/O
#define W25Q_FULLID_QUAD_IO 0x94U			///< read Manufacturer ID & Device ID by quad I/O
#define W25Q_READ_UID 0x4BU					///< read unique chip 64-bit ID
#define W25Q_READ_JEDEC_ID 0x9FU			///< read JEDEC-standard ID
#define W25Q_READ_SFDP 0x5AU				///< read SFDP register parameters
#define W25Q_ERASE_SECURITY_REG 0x44U		///< erase security registers
#define W25Q_PROG_SECURITY_REG 0x42U		///< program security registers
#define W25Q_READ_SECURITY_REG 0x48U		///< read security registers
#define W25Q_IND_BLOCK_LOCK 0x36U			///< make block/sector read-only
#define W25Q_IND_BLOCK_UNLOCK 0x39U			///< disable block/sector protection
#define W25Q_READ_BLOCK_LOCK 0x3DU			///< check block/sector protection
#define W25Q_GLOBAL_LOCK 0x7EU				///< global read-only protection enable
#define W25Q_GLOBAL_UNLOCK 0x98U			///< global read-only protection disable
#define W25Q_ENABLE_RST 0x66U				///< enable software-reset ability
#define W25Q_RESET 0x99U					///< make software reset
/// @}

/// @}

#ifdef __cplusplus
}
#endif

#endif /* W25Q_QSPI_W25Q_MEM_H_ */
