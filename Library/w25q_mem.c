/**
 *******************************************
 * @file    w25q_mem.c
 * @author  Dmitriy Semenov / Crazy_Geeks
 * @version 0.1b
 * @date    12-August-2021
 * @brief   Source file for W25Qxxx lib
 * @note    https://github.com/Crazy-Geeks/STM32-W25Q-QSPI
 *******************************************
 *
 * @note https://ru.mouser.com/datasheet/2/949/w25q256jv_spi_revg_08032017-1489574.pdf
 * @note https://www.st.com/resource/en/application_note/DM00227538-.pdf
 */

/**
 * @addtogroup W25Q_Driver
 * @{
 */

#include "w25q_mem.h"

/**
 * @addtogroup W25Q_Exp Exported types
 * @brief External fields and data
 * @{
 */
extern QSPI_HandleTypeDef hqspi;	///< Quad SPI HAL Instance
/// @}

/**
 * @addtogroup W25Q_PrivFi Private fields
 * @brief Private variables and defines
 * @{
 */
#define w25q_delay(x) HAL_Delay(x) 	///< Delay define to provide future support of RTOS
W25Q_STATUS_REG w25q_status; 		///< Internal status structure instance

/// @}

/**
 * @addtogroup W25Q_PrivFu Private Methods
 * @brief Internal lib's functions
 * @{
 */
W25Q_STATE W25Q_WriteEnable(bool enable); 	///< Toggle WOL bit
W25Q_STATE W25Q_EnableQSPI(bool enable);	///< Toggle QE bit
W25Q_STATE W25Q_Enter4ByteMode(bool enable); 	///< Toggle ADS bit
W25Q_STATE W25Q_SetExtendedAddr(u8_t Addr);  	///< Set addr in 3-byte mode
W25Q_STATE W25Q_GetExtendedAddr(u8_t *outAddr); ///< Get addr in 3-byte mode

static inline u32_t page_to_addr(u32_t pageNum, u8_t pageShift); ///< Translate page addr to byte addr
/// @}

/**
 * @addtogroup W25Q_Pub Public methods
 * @brief Public Methods
 * @{
 */

/**
 * @addtogroup W25Q_Init Init methods
 * @brief Initalization
 * @{
 */

/**
 * @brief W25Q Init function
 *
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_Init(void) {
	W25Q_STATE state;		// temp status variable

	// read id
	u8_t id = 0;
	state = W25Q_ReadID(&id);
	if (state != W25Q_OK)
		return state;
	// u can check id here

	// read chip's state to private lib's struct
	state = W25Q_ReadStatusStruct(NULL);
	if (state != W25Q_OK)
		return state;

#if MEM_FLASH_SIZE > 128 // if 4-byte mode

	/* If power-default 4-byte
	 mode disabled */
	if (!w25q_status.ADP) {
		u8_t buf_reg = 0;
		state = W25Q_ReadStatusReg(&buf_reg, 3);
		if (state != W25Q_OK)
			return state;
		buf_reg |= 0b10; 	// set ADP bit
		state = W25Q_WriteStatusReg(buf_reg, 3);
		if (state != W25Q_OK)
			return state;
	}

	/* If current 4-byte
	 mode disabled */
	if (!w25q_status.ADS) {
		state = W25Q_Enter4ByteMode(1);
		if (state != W25Q_OK)
			return state;
	}
#endif

	/* If Quad-SPI mode disabled */
	if (!w25q_status.QE) {
		u8_t buf_reg = 0;
		state = W25Q_ReadStatusReg(&buf_reg, 2);
		if (state != W25Q_OK)
			return state;
		buf_reg |= 0b10;
		state = W25Q_WriteStatusReg(buf_reg, 2);
		if (state != W25Q_OK)
			return state;
	}

	// make another read
	state = W25Q_ReadStatusStruct(NULL);
	// return communication status
	return state;
}

/**
 * @}
 * @addtogroup W25Q_Reg Register Functions
 * @brief Operations with Status Registers
 * @{
 */

/**
 * @brief W25Q Enable Volatile SR
 * Makes status register volatile (temporary)
 *
 * @attention Func in development
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_EnableVolatileSR(void) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Read Status Register
 * Read one status register
 *
 * @param[out] reg_data 1 byte
 * @param[in] reg_num Desired register 1..3
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadStatusReg(u8_t *reg_data, u8_t reg_num) {
	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...

	if (reg_num == 1)
		com.Instruction = W25Q_READ_SR1;
	else if (reg_num == 2)
		com.Instruction = W25Q_READ_SR2;
	else if (reg_num == 3)
		com.Instruction = W25Q_READ_SR3;
	else
		return W25Q_PARAM_ERR;

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_1_LINE;
	com.NbData = 1;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	if (HAL_QSPI_Receive(&hqspi, reg_data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}

	return W25Q_OK;
}

/**
 * @brief W25Q Write Status Register
 * Write one status register
 *
 * @param[in] reg_data 1 byte
 * @param[in] reg_num Desired register 1..3
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_WriteStatusReg(u8_t reg_data, u8_t reg_num) {
	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	W25Q_STATE state = W25Q_WriteEnable(1);
	if (state != W25Q_OK)
		return state;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...

	if (reg_num == 1)
		com.Instruction = W25Q_WRITE_SR1;
	else if (reg_num == 2)
		com.Instruction = W25Q_WRITE_SR2;
	else if (reg_num == 3)
		com.Instruction = W25Q_WRITE_SR3;
	else
		return W25Q_PARAM_ERR;

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_1_LINE;
	com.NbData = 1;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	if (HAL_QSPI_Transmit(&hqspi, &reg_data, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}

	return W25Q_OK;
}

/**
 * @brief W25Q Read Status Registers
 * Read all status registers to struct
 *
 * @param[out] status W25Q_STATUS_REG Pointer
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadStatusStruct(W25Q_STATUS_REG *status) {
	// buffer enum-variable
	W25Q_STATE state;

	// buffer register variables
	u8_t SRs[3] = { 0, };

	// first portion
	state = W25Q_ReadStatusReg(&SRs[0], 1);
	if (state != W25Q_OK)
		return state;

	// second portion
	state = W25Q_ReadStatusReg(&SRs[1], 2);
	if (state != W25Q_OK)
		return state;

	// third portion
	state = W25Q_ReadStatusReg(&SRs[2], 3);
	if (state != W25Q_OK)
		return state;
	if(status){
		status->BUSY = w25q_status.BUSY = SRs[0] & 0b1;
		status->WEL = w25q_status.WEL = (SRs[0] >> 1) & 0b1;
		status->QE = w25q_status.QE = (SRs[1] >> 1) & 0b1;
		status->SUS = w25q_status.SUS = (SRs[1] >> 7) & 0b1;
		status->ADS = w25q_status.ADS = SRs[2] & 0b1;
		status->ADP = w25q_status.ADP = (SRs[2] >> 1) & 0b1;
		status->SLEEP = w25q_status.SLEEP; // возможно нужно вынести в начало (тестить)
	}

	return state;
}

/**
 * @brief W25Q Check Busy flag
 * Fast checking Busy flag
 *
 * @param none
 * @return W25Q_STATE enum (W25Q_OK / W25Q_BUSY)
 */
W25Q_STATE W25Q_IsBusy(void) {
	W25Q_STATE state;
	u8_t sr = 0;

	state = W25Q_ReadStatusReg(&sr, 1);
	if (state != W25Q_OK)
		return state;

	w25q_status.BUSY = sr & 0b1;

	return w25q_status.BUSY ? W25Q_BUSY : W25Q_OK;
}

/**
 * @}
 * @addtogroup W25Q_Read Read Functions
 * @brief Read operations - single data type variables or raw 8-bit blocks
 * @{
 */

/**
 * @brief W25Q Read single Signed Byte
 * Read signed 8-bit byte variable
 *
 * @param[out] buf Data to be read (single)
 * @param[in] pageShift Byte shift inside page (0..255)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadSByte(i8_t *buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data;
	W25Q_STATE state = W25Q_ReadRaw(&data, 1, rawAddr);
	if (state != W25Q_OK)
		return state;
	memcpy(buf, &data, 1);
	return W25Q_OK;
}

/**
 * @brief W25Q Read single Unsigned Byte
 * Read unsigned 8-bit byte variable
 *
 * @param[out] buf Data to be read (single)
 * @param[in] pageShift Byte shift inside page (0..255)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadByte(u8_t *buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data;
	W25Q_STATE state = W25Q_ReadRaw(&data, 1, rawAddr);
	if (state != W25Q_OK)
		return state;
	buf[0] = data;
	return W25Q_OK;
}

/**
 * @brief W25Q Read single Signed Word
 * Read signed 16-bit word variable
 *
 * @param[out] buf Data to be read (single)
 * @param[in] pageShift Byte shift inside page (0..254)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadSWord(i16_t *buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 2)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[2];
	W25Q_STATE state = W25Q_ReadRaw(data, 2, rawAddr);
	if (state != W25Q_OK)
		return state;
	memcpy(buf, data, 2);
	return W25Q_OK;
}

/**
 * @brief W25Q Read single Unsigned Word
 * Read unsigned 16-bit word variable
 *
 * @param[out] buf Data to be read (single)
 * @param[in] pageShift Byte shift inside page (0..254)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadWord(u16_t *buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 2)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[2];
	W25Q_STATE state = W25Q_ReadRaw(data, 2, rawAddr);
	if (state != W25Q_OK)
		return state;
	memcpy(buf, data, 2);
	return W25Q_OK;
}

/**
 * @brief W25Q Read single Signed Long
 * Read signed 32-bit long variable
 *
 * @param[out] buf Data to be read (single)
 * @param[in] pageShift Byte shift inside page (0..252)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadSLong(i32_t *buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 4)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[4];
	W25Q_STATE state = W25Q_ReadRaw(data, 4, rawAddr);
	if (state != W25Q_OK)
		return state;
	memcpy(buf, data, 4);
	return W25Q_OK;
}

/**
 * @brief W25Q Read single Signed Long
 * Read signed 32-bit long variable
 *
 * @param[out] buf Data to be read (single)
 * @param[in] pageShift Byte shift inside page (0..252)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadLong(u32_t *buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 4)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[4];
	W25Q_STATE state = W25Q_ReadRaw(data, 4, rawAddr);
	if (state != W25Q_OK)
		return state;
	memcpy(buf, data, 4);
	return W25Q_OK;
}

/**
 * @brief W25Q Read any 8-bit data
 * Read any 8-bit data from preffered page place
 *
 * @note Use memcpy to decode data
 * @param[out] buf Pointer to data to be read (single or array)
 * @param[in] len Length of data (1..256)
 * @param[in] pageShift Byte shift inside page (0..256 - len)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadData(u8_t *buf, u16_t len, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || len == 0 || len > 256 || pageShift > 256 - len)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	return W25Q_ReadRaw(buf, len, rawAddr);
}

/**
 * @brief W25Q Read any 8-bit data from raw addr
 * Read any 8-bit data from preffered chip address
 *
 * @note Address is in [byte] size
 * @note Be carefull with page overrun
 * @param[out] buf Pointer to data to be written (single or array)
 * @param[in] data_len Length of data (1..256)
 * @param[in] rawAddr Start address of chip's cell
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadRaw(u8_t *buf, u16_t data_len, u32_t rawAddr) {
	if (data_len > 256 || data_len == 0)
		return W25Q_PARAM_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
#if MEM_FLASH_SIZE > 128U
	com.Instruction = W25Q_FAST_READ_QUAD_IO_4B;	 // Command
	com.AddressSize = QSPI_ADDRESS_32_BITS;
#else
		com.Instruction = W25Q_FAST_READ_QUAD_IO;	 // Command
		com.AddressSize = QSPI_ADDRESS_24_BITS;
	#endif
	com.AddressMode = QSPI_ADDRESS_4_LINES;

	com.Address = rawAddr;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 6;
	com.DataMode = QSPI_DATA_4_LINES;
	com.NbData = data_len;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	if (HAL_QSPI_Receive(&hqspi, buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return W25Q_SPI_ERR;

	return W25Q_OK;
}

/**
 * @brief W25Q Read any 8-bit data from raw addr
 * Read any 8-bit data from preffered chip address by SINGLE SPI
 *
 * @note Works only with SINGLE SPI Line
 * @param[out] buf Pointer to data array
 * @param[in] len Length of array
 * @param[in] Addr Address to data
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_SingleRead(u8_t *buf, u32_t len, u32_t Addr) {
	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
#if MEM_FLASH_SIZE > 128U
	com.Instruction = W25Q_READ_DATA_4B;	 // Command
	com.AddressSize = QSPI_ADDRESS_32_BITS;
#else
	com.Instruction = W25Q_READ_DATA;	 // Command
	com.AddressSize = QSPI_ADDRESS_24_BITS;
#endif
	com.AddressMode = QSPI_ADDRESS_1_LINE;

	com.Address = Addr;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_1_LINE;
	com.NbData = len;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	if (HAL_QSPI_Receive(&hqspi, buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
		return W25Q_SPI_ERR;

	return W25Q_OK;
}

/**
 * @}
 * @addtogroup W25Q_Write Write functions
 * @brief Program/write operations - single data type variables or raw 8-bit blocks
 * @{
 */

/**
 * @brief W25Q Burst Wrap settings
 *
 * @attention Func in development
 *
 * @param[in] WrapSize Wrap size: 8/16/32/64 / 0 - disable
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_SetBurstWrap(u8_t WrapSize) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Program single Signed Byte
 * Program signed 8-bit byte variable
 *
 * @param[in] buf Data to be written (single)
 * @param[in] pageShift Byte shift inside page (0..255)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramSByte(i8_t buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data;
	memcpy(&data, &buf, 1);
	return W25Q_ProgramRaw(&data, 1, rawAddr);
}

/**
 * @brief W25Q Program single Unsigned Byte
 * Program unsigned 8-bit byte vairable
 *
 * @param[in] buf Data to be written (single)
 * @param[in] pageShift Byte shift inside page (0..255)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramByte(u8_t buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data;
	memcpy(&data, &buf, 1);
	return W25Q_ProgramRaw(&data, 1, rawAddr);
}

/**
 * @brief W25Q Program single Signed Word
 * Program signed 16-bit word vairable
 *
 * @param[in] buf Data to be written (single)
 * @param[in] pageShift Byte shift inside page (0..254)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramSWord(i16_t buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 2)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[2];
	memcpy(data, &buf, 2);
	return W25Q_ProgramRaw(data, 2, rawAddr);
}

/**
 * @brief W25Q Program single Unsigned Word
 * Program unsigned 16-bit word vairable
 *
 * @param[in] buf Data to be written (single)
 * @param[in] pageShift Byte shift inside page (0..254)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramWord(u16_t buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 2)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[2];
	memcpy(data, &buf, 2);
	return W25Q_ProgramRaw(data, 2, rawAddr);
}

/**
 * @brief W25Q Program single Signed Long
 * Program signed 32-bit long vairable
 *
 * @param[in] buf Data to be written (single)
 * @param[in] pageShift Byte shift inside page (0..252)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramSLong(i32_t buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 4)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[4];
	memcpy(data, &buf, 4);
	return W25Q_ProgramRaw(data, 4, rawAddr);
}

/**
 * @brief W25Q Program single Unigned Long
 * Program unsigned 32-bit long vairable
 *
 * @param[in] buf Data to be written (single)
 * @param[in] pageShift Byte shift inside page (0..252)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramLong(u32_t buf, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || pageShift > 256 - 4)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	u8_t data[4];
	memcpy(data, &buf, 4);
	return W25Q_ProgramRaw(data, 4, rawAddr);
}

/**
 * @brief W25Q Program any 8-bit data
 * Program any 8-bit data to preffered page place
 *
 * @note Use memcpy to prepare data
 * @param[in] buf Pointer to data to be written (single or array)
 * @param[in] len Length of data (1..256)
 * @param[in] pageShift Byte shift inside page (0..256 - len)
 * @param[in] pageNum Page number (0..PAGE_COUNT-1)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramData(u8_t *buf, u16_t len, u8_t pageShift, u32_t pageNum) {
	if (pageNum >= PAGE_COUNT || len == 0 || len > 256 || pageShift > 256 - len)
		return W25Q_PARAM_ERR;
	u32_t rawAddr = page_to_addr(pageNum, pageShift);
	return W25Q_ProgramRaw(buf, len, rawAddr);
}

/**
 * @brief W25Q Program any 8-bit data to raw addr
 * Program any 8-bit data to preffered chip address
 *
 * @note Address is in [byte] size
 * @note Be carefull with page overrun
 * @param[in] buf Pointer to data to be written (single or array)
 * @param[in] data_len Length of data (1..256)
 * @param[in] rawAddr Start address of chip's cell
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgramRaw(u8_t *buf, u16_t data_len, u32_t rawAddr) {
	if (data_len > 256 || data_len == 0)
		return W25Q_PARAM_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	W25Q_STATE state = W25Q_WriteEnable(1);
	if (state != W25Q_OK)
		return state;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
#if MEM_FLASH_SIZE > 128U
	com.Instruction = W25Q_PAGE_PROGRAM_QUAD_INP_4B;	 // Command
	com.AddressSize = QSPI_ADDRESS_32_BITS;
#else
		com.Instruction = W25Q_PAGE_PROGRAM_QUAD_INP;	 // Command
		com.AddressSize = QSPI_ADDRESS_24_BITS;
	#endif
	com.AddressMode = QSPI_ADDRESS_1_LINE;

	com.Address = rawAddr;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_4_LINES;
	com.NbData = data_len;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	if (HAL_QSPI_Transmit(&hqspi, buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	return W25Q_OK;
}

/**
 * @}
 * @addtogroup W25Q_Erase Erase functions
 * @brief Erase sector, blocks or whole chip
 * @{
 */

/**
 * @brief W25Q Sector erase (4KB)
 * Minimal size operation to erase data
 *
 * @note Should be executed before writing
 * @param[in] SectAddr Sector start address
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_EraseSector(u32_t SectAddr) {
	if (SectAddr >= SECTOR_COUNT)
		return W25Q_PARAM_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	u32_t rawAddr = SectAddr * MEM_SECTOR_SIZE * 1024U;

	W25Q_STATE state = W25Q_WriteEnable(1);
	if (state != W25Q_OK)
		return state;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
#if MEM_FLASH_SIZE > 128U
	com.Instruction = W25Q_SECTOR_ERASE_4B;	 // Command
	com.AddressSize = QSPI_ADDRESS_32_BITS;
#else
	com.Instruction = W25Q_SECTOR_ERASE;	 // Command
	com.AddressSize = QSPI_ADDRESS_24_BITS;
#endif
	com.AddressMode = QSPI_ADDRESS_1_LINE;

	com.Address = rawAddr;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	return W25Q_OK;
}

/**
 * @brief W25Q Block erase (32/64 KB)
 * Func to erase big block
 *
 * @note Should be executed before writing
 * @param[in] BlockAddr Block start address
 * @param[in] size Size of block: 32KB or 64KB
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_EraseBlock(u32_t BlockAddr, u8_t size) {
	if (size != 32 && size != 64)
		return W25Q_PARAM_ERR;
	if ((size == 64 && BlockAddr >= BLOCK_COUNT)
			|| (size == 32 && BlockAddr >= BLOCK_COUNT * 2))
		return W25Q_PARAM_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	u32_t rawAddr = BlockAddr * MEM_SECTOR_SIZE * 1024U * 16;
	if (size == 32)
		rawAddr /= 2;

	W25Q_STATE state = W25Q_WriteEnable(1);
	if (state != W25Q_OK)
		return state;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...

	if (size == 32) {
		com.Instruction = W25Q_32KB_BLOCK_ERASE;	 // Command
#if MEM_FLASH_SIZE > 128U
		com.AddressSize = QSPI_ADDRESS_32_BITS;
#else
	com.AddressSize = QSPI_ADDRESS_24_BITS;
#endif
	} else if (size == 64) {
#if MEM_FLASH_SIZE > 128U
		com.Instruction = W25Q_64KB_BLOCK_ERASE_4B;	 // Command
		com.AddressSize = QSPI_ADDRESS_32_BITS;
#else
	com.Instruction = W25Q_64KB_BLOCK_ERASE;	 // Command
	com.AddressSize = QSPI_ADDRESS_24_BITS;
#endif
	}

	com.AddressMode = QSPI_ADDRESS_1_LINE;

	com.Address = rawAddr;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	return W25Q_OK;
}

/**
 * @brief W25Q Chip erase
 * Func to erase all the data on chip
 *
 * @note Should be executed before writing
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_EraseChip(void) {
	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	W25Q_STATE state = W25Q_WriteEnable(1);
	if (state != W25Q_OK)
		return state;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_CHIP_ERASE;	 // Command

	com.AddressSize = QSPI_ADDRESS_NONE;
	com.AddressMode = QSPI_ADDRESS_NONE;
	com.Address = 0;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	return W25Q_OK;
}

/**
 * @}
 * @addtogroup W25Q_SUS Suspend functions
 * @brief Pause/resume operations
 * @{
 */

/**
 * @brief W25Q Programm/Erase Suspend
 * Pause programm or suspend operatiom
 *
 * @note SUS == 0 && BUSY == 1, otherwise ignored
 * @note Power loose during suspend state may corrupt data
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgSuspend(void) {
	if (W25Q_BUSY != W25Q_IsBusy())
		return W25Q_CHIP_IGNORE;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_ERASEPROG_SUSPEND;	 // Command

	com.AddressSize = QSPI_ADDRESS_NONE;
	com.AddressMode = QSPI_ADDRESS_NONE;
	com.Address = 0;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	return W25Q_OK;
}

/**
 * @brief W25Q Programm/Erase Resume
 * Resume suspended state
 *
 * @note SUS == 1, otherwise ignored
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgResume(void) {

	W25Q_STATE state = W25Q_ReadStatusStruct(NULL);
	if (state != W25Q_OK)
		return state;

	if (w25q_status.SUS != 1)
		return W25Q_CHIP_IGNORE;

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_ERASEPROG_RESUME;	 // Command

	com.AddressSize = QSPI_ADDRESS_NONE;
	com.AddressMode = QSPI_ADDRESS_NONE;
	com.Address = 0;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK)
		return W25Q_SPI_ERR;

	return W25Q_OK;
}

/**
 * @}
 * @addtogroup W25Q_Sleep Sleep functions
 * @brief Баю-бай, ток засыпай
 * @{
 */

/**
 * @brief W25Q Sleep / Power Down
 * Set chip to low-power state
 *
 * @note Use WakeUP or ReadID
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_Sleep(void) {
	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_POWERDOWN;	 // Command

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	w25q_delay(1); // Give a little time to sleep

	w25q_status.SLEEP = 1;

	return W25Q_OK;
}

/**
 * @brief W25Q WakeUP
 * Wake UP function
 *
 * @param none
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_WakeUP(void) {
	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_POWERUP;	 // Command

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	w25q_delay(1); // Give a little time to wake

	return W25Q_OK;
}

/**
 * @}
 * @addtogroup W25Q_ID ID functions
 * @brief Who am I? Хто я?
 * @{
 */

/**
 * @brief W25Q Read ID
 * Function for reading chip ID
 *
 * @param[out] buf Pointer to output data (1 byte)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadID(u8_t *buf) {
	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_DEVID;	 // Command

	com.AddressMode = QSPI_ADDRESS_1_LINE;
	com.AddressSize = QSPI_ADDRESS_24_BITS;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_1_LINE;
	com.NbData = 1;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	if (HAL_QSPI_Receive(&hqspi, buf, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	return W25Q_OK;
}

/**
 * @brief W25Q Read chip Full ID
 * Read Manufacturer ID + Device ID
 *
 * @attention Func in development
 * @param[out] buf Pointer to data from ID register
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadFullID(u8_t *buf) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Read chip UID
 * Read Unique ID
 *
 * @attention Func in development
 * @param[out] buf Pointer to data from ID register
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadUID(u8_t *buf) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Read JEDEC ID
 * Read ID by JEDEC standards
 *
 * @attention Func in development
 * @param[out] buf Pointer to data from ID register
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadJEDECID(u8_t *buf) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Read SFDP Register
 * Read device descriptor by SFDP standard
 *
 * @attention Func in development
 * @param[out] buf Pointer to data from ID register
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadSFDPRegister(u8_t *buf) {
	return W25Q_PARAM_ERR;
}

/**
 * @}
 * @addtogroup W25Q_Secure Security register functions
 * @brief Shhh, security
 * @{
 */

/**
 * @brief W25Q Erase Security Registers
 * Clean security registers (one or all)
 *
 * @attention Func in development
 * @param[in] numReg Number of security register (1..3 / 0-all)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_EraseSecurityRegisters(u8_t numReg) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Program Security Register
 * Write data to security reg
 *
 * @attention Func in development
 * @param[in] buf Pointer to 8-bit data bufer
 * @param[in] numReg Number of security register (1..3)
 * @param[in] byteAddr Byte addr in register (0..255)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ProgSecurityRegisters(u8_t *buf, u8_t numReg, u8_t byteAddr) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Read Security Register
 * Read data from security reg
 *
 * @attention Func in development
 * @param[out] buf Pointer to 8-bit data bufer
 * @param[in] numReg Number of security register (1..3)
 * @param[in] byteAddr Byte addr in register (0..255)
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_ReadSecurityRegisters(u8_t *buf, u8_t numReg, u8_t byteAddr) {
	return W25Q_PARAM_ERR;
}

/**
 * @}
 * @addtogroup W25Q_Protect Read-only protection functions
 * @brief No writing! Protect block or whole chip
 * @{
 */

/**
 * @brief W25Q 4K Block lock
 * Set read-only status to 4K block
 *
 * @attention Func in development
 * @param[in] Addr Block address
 * @param[in] enable 1-Enable/0-Disable
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_BlockReadOnly(u32_t Addr, bool enable) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q 4K Block lock CHECK
 * Check read-only status from 4K block
 *
 * @attention Func in development
 * @param[out] state Block read-only status (1-locked/0-unlocked)
 * @param[in] Addr Block address
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_BlockReadOnlyCheck(bool *state, u32_t Addr) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Global read-only lock
 * Set read-only status to the whole chip
 *
 * @attention Func in development
 * @param[in] enable 1-enable/0-disable
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_GlobalReadOnly(bool enable){
	return W25Q_PARAM_ERR;
}

/**
 * @}
 * @addtogroup W25Q_RST Reset functions
 * @brief Reboot the chip
 * @{
 */

/**
 * @brief W25Q Software Reset
 * Reset by register (not by external GPIO pin)
 *
 * @param[in] force Enable/disable (0/1) force reset - wait for BUSY and SUSpend
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_SwReset(bool force) {

	W25Q_STATE state;	// temp status reg

	state = W25Q_ReadStatusStruct(NULL); // read settings
	if (state != W25Q_OK)
		return state;

	if (!force || w25q_status.BUSY || w25q_status.SUS) // if busy or suspend
		return W25Q_CHIP_ERR;

	if (force) {
		while (W25Q_IsBusy() == W25Q_BUSY)
			w25q_delay(1);
		if (w25q_status.SUS)
			W25Q_ProgResume();
	}

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = W25Q_ENABLE_RST;	 // Command

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	w25q_delay(1); // Give a little time to prepare

	com.Instruction = W25Q_RESET;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	w25q_delay(5); // Give a little time to reset

	state = W25Q_Init();

	return state;
}
/// @}
// addgroup{
/// @}
// Public {

/**
 * @brief W25Q Toggle WEL bit
 * Toggle write enable latch
 *
 * @param[in] enable 1-enable write/0-disable write
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_WriteEnable(bool enable) {
	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = enable ? W25Q_WRITE_ENABLE : W25Q_WRITE_DISABLE;

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}
	w25q_delay(1); // Give a little time to sleep

	w25q_status.WEL = 1;

	return W25Q_OK;
}

/**
 * @brief W25Q Toggle 4-byte mode
 *
 * @note Affects only ADS bit
 * @param[in] enable 1-enable/0-disable
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_Enter4ByteMode(bool enable) {

	while (W25Q_IsBusy() == W25Q_BUSY)
		w25q_delay(1);

	QSPI_CommandTypeDef com;

	com.InstructionMode = QSPI_INSTRUCTION_1_LINE; // QSPI_INSTRUCTION_...
	com.Instruction = enable ? W25Q_ENABLE_4B_MODE : W25Q_DISABLE_4B_MODE;

	com.AddressMode = QSPI_ADDRESS_NONE;
	com.AddressSize = QSPI_ADDRESS_NONE;
	com.Address = 0x0U;

	com.AlternateByteMode = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytes = QSPI_ALTERNATE_BYTES_NONE;
	com.AlternateBytesSize = QSPI_ALTERNATE_BYTES_NONE;

	com.DummyCycles = 0;
	com.DataMode = QSPI_DATA_NONE;
	com.NbData = 0;

	com.DdrMode = QSPI_DDR_MODE_DISABLE;
	com.DdrHoldHalfCycle = QSPI_DDR_HHC_ANALOG_DELAY;
	com.SIOOMode = QSPI_SIOO_INST_EVERY_CMD;

	if (HAL_QSPI_Command(&hqspi, &com, HAL_QSPI_TIMEOUT_DEFAULT_VALUE)
			!= HAL_OK) {
		return W25Q_SPI_ERR;
	}

	w25q_delay(1); // Give a little time to set command

	return W25Q_ReadStatusStruct(NULL);
}

/**
 * @brief W25Q Set extended byte
 * Extended byte in 3-byte mode
 *
 * @attention Func in development
 * @param[in] Addr 4th byte of addr
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_SetExtendedAddr(u8_t Addr) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief W25Q Get extended byte
 * Read extended byte in 3-byte mode
 *
 * @attention Func in development
 * @param[out] Addr 4th byte of addr
 * @return W25Q_STATE enum
 */
W25Q_STATE W25Q_GetExtendedAddr(u8_t *outAddr) {
	return W25Q_PARAM_ERR;
}

/**
 * @brief Page to address
 * Translate page to byte-address
 *
 * @param[in] pageNum Number of page
 * @param[in] pageShift Byte to shift inside page
 * @return byte-address
 */
u32_t page_to_addr(u32_t pageNum, u8_t pageShift) {
	return pageNum * MEM_PAGE_SIZE + pageShift;
}

///@}
