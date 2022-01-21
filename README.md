## W25Qxxx QSPI STM32 Library

### HAL STM32 Driver for Winbond W25Q-series memory, using Quad-SPI interface
![Poster](/Resources/Poster.png)

### Features:
- Easy applying: ST's HAL functions are used 
- Uses Quad-SPI interface (4 lines)
- Supports *(almost)* all chip commands *(later)*
- Simple usage with data types wrapper-functions, or use raw 8-bit data

### Documentation is also available in [doxygen](https://crazy-geeks.github.io/STM32-W25Q-QSPI/)

### Function reference (from .h file):
```c
W25Q_STATE W25Q_Init(void);		// Initalize function

W25Q_STATE W25Q_ReadStatusReg(u8_t *reg_data, u8_t reg_num); // Read status register to variable
W25Q_STATE W25Q_WriteStatusReg(u8_t reg_data, u8_t reg_num); // Write status register from variable
W25Q_STATE W25Q_ReadStatusStruct(W25Q_STATUS_REG *status);	 // Read all status registers to struct
W25Q_STATE W25Q_IsBusy(void);	// Check chip's busy status

W25Q_STATE W25Q_ReadSByte(i8_t *buf, u8_t pageShift, u32_t pageNum);	// Read signed 8-bit variable
W25Q_STATE W25Q_ReadByte(u8_t *buf, u8_t pageShift, u32_t pageNum);		// Read 8-bit variable
W25Q_STATE W25Q_ReadSWord(i16_t *buf, u8_t pageShift, u32_t pageNum);	// Read signed 16-bit variable
W25Q_STATE W25Q_ReadWord(u16_t *buf, u8_t pageShift, u32_t pageNum);	// Read 16-bit variable
W25Q_STATE W25Q_ReadSLong(i32_t *buf, u8_t pageShift, u32_t pageNum);	// Read signed 32-bit variable
W25Q_STATE W25Q_ReadLong(u32_t *buf, u8_t pageShift, u32_t pageNum);	// Read 32-bit variable
W25Q_STATE W25Q_ReadData(u8_t *buf, u16_t len, u8_t pageShift, u32_t pageNum);  // Read any 8-bit data
W25Q_STATE W25Q_ReadRaw(u8_t *buf, u16_t data_len, u32_t rawAddr);  // Read data from raw addr
W25Q_STATE W25Q_SingleRead(u8_t *buf, u32_t len, u32_t Addr);	 // Read data from raw addr by single line

W25Q_STATE W25Q_EraseSector(u32_t SectAddr);  // Erase 4KB Sector
W25Q_STATE W25Q_EraseBlock(u32_t BlockAddr, u8_t size); // Erase 32KB/64KB Sector
W25Q_STATE W25Q_EraseChip(void);  // Erase all chip

W25Q_STATE W25Q_ProgramSByte(i8_t buf, u8_t pageShift, u32_t pageNum);	// Program signed 8-bit variable
W25Q_STATE W25Q_ProgramByte(u8_t buf, u8_t pageShift, u32_t pageNum);  // Program 8-bit variable
W25Q_STATE W25Q_ProgramSWord(i16_t buf, u8_t pageShift, u32_t pageNum);	// Program signed 16-bit variable
W25Q_STATE W25Q_ProgramWord(u16_t buf, u8_t pageShift, u32_t pageNum);	// Program 16-bit variable
W25Q_STATE W25Q_ProgramSLong(i32_t buf, u8_t pageShift, u32_t pageNum);	// Program signed 32-bit variable
W25Q_STATE W25Q_ProgramLong(u32_t buf, u8_t pageShift, u32_t pageNum);	// Program 32-bit variable
W25Q_STATE W25Q_ProgramData(u8_t *buf, u16_t len, u8_t pageShift, u32_t pageNum); // Program any 8-bit data
W25Q_STATE W25Q_ProgramRaw(u8_t *buf, u16_t data_len, u32_t rawAddr); 	// Program data to raw addr

W25Q_STATE W25Q_ProgSuspend(void); // Pause Programm/Erase operation
W25Q_STATE W25Q_ProgResume(void); // Resume Programm/Erase operation

W25Q_STATE W25Q_Sleep(void);	// Set low current consumption
W25Q_STATE W25Q_WakeUP(void);	// Wake the chip up from sleep mode

W25Q_STATE W25Q_ReadID(u8_t *buf);  // Read chip ID

W25Q_STATE W25Q_SwReset(bool force);	// Software reset
```
### Functions that aren't yet ready:
```c
W25Q_STATE W25Q_EnableVolatileSR(void);  // Make Status Register Volatile
W25Q_STATE W25Q_SetBurstWrap(u8_t WrapSize); // Set Burst with Wrap
W25Q_STATE W25Q_ReadFullID(u8_t *buf);  // Read full chip ID (Manufacturer ID + Device ID)
W25Q_STATE W25Q_ReadUID(u8_t *buf);     // Read unique chip ID
W25Q_STATE W25Q_ReadJEDECID(u8_t *buf); // Read ID by JEDEC Standards
W25Q_STATE W25Q_ReadSFDPRegister(u8_t *buf); // Read device descriptor (SFDP Standard)
W25Q_STATE W25Q_EraseSecurityRegisters(u8_t numReg);	// Erase security register
W25Q_STATE W25Q_ProgSecurityRegisters(u8_t *buf, u8_t numReg, u8_t byteAddr);	// Program security register
W25Q_STATE W25Q_ReadSecurityRegisters(u8_t *buf, u8_t numReg, u8_t byteAddr);	// Read security register
W25Q_STATE W25Q_BlockReadOnly(u32_t Addr, bool enable);   // Individual block/sector read-only lock
W25Q_STATE W25Q_BlockReadOnlyCheck(bool *state, u32_t Addr);  // Check block's/sector's read-only lock status
W25Q_STATE W25Q_GlobalReadOnly(bool enable);		// Set read-only param to all chip
```

### Instructions for use:
- Use *CubeMX* to configure *QUADSPI* peripheral reffer to your datasheet 
- Memory size calculation *([AN4760](/Datasheets/AN4760-QSPI.pdf) page 45)*:<br/>
2^(N+1) = Mem size in **bytes**<br/>
Example: *256 Mbit* = *32 MByte* = *32'768 KByte* = *33'554'432 Byte* = *2^25 Byte* => N = **24**<br/>
![Flash size](/Resources/FSize.png)
- Connect memory to STM reffer to [Datasheet](/Datasheets/winbond_w25q256jv.pdf), or your's chip datasheet
- Include "w25q_mem.h" to your code 
- Start with Init function
- Enjoy )

**Any questions? Write an issue! Or create pull request.** 

**Donate:** [PayPal](https://paypal.me/yasnosos ) / [DonationAlerts](https://www.donationalerts.com/r/yasnosos )

<br/>

### Example

![Example](/Resources/Example_result.png)
