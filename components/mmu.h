/*
 * mmu.h
 *
 * Defines the constants for the Game Boy memory map and declares
 * the external memory arrays and MMU access function prototypes.
 *
 * Created on: 12 Jul 2025
 * Author: hawke
 */



#ifndef COMPONENTS_MMU_H_
#define COMPONENTS_MMU_H_

#include <stdint.h>

// ----------------------------------------------------------------------
// MMU Address and Size Constants
// ----------------------------------------------------------------------

// ROM Bank 00 (Fixed bank, 16 KiB)
#define MMU_ADDRESS_ROM_BANK_00_START 		(0x0000)
#define MMU_ADDRESS_ROM_BANK_00_END 		(0x3FFF)
#define MMU_ROM_BANK_00_SIZE				(MMU_ADDRESS_ROM_BANK_00_END - MMU_ADDRESS_ROM_BANK_00_START + 1)

// ROM Bank 01–NN (Switchable bank, 16 KiB)
#define MMU_ADDRESS_ROM_BANK_01_NN_START 	(0x4000)
#define MMU_ADDRESS_ROM_BANK_01_NN_END 		(0x7FFF)
#define MMU_ROM_BANK_01_SIZE				(MMU_ADDRESS_ROM_BANK_01_NN_END - MMU_ADDRESS_ROM_BANK_01_NN_START + 1)

// Combined ROM region end address
#define MMU_ADDRESS_ROM_BANK_END			(0x7FFF)

// Video RAM (VRAM) (8 KiB, switchable bank 0/1 in CGB)
#define MMU_ADDRESS_V_RAM_START 			(0x8000)
#define MMU_ADDRESS_V_RAM_END 				(0x9FFF)
#define MMU_V_RAM_SIZE						(MMU_ADDRESS_V_RAM_END - MMU_ADDRESS_V_RAM_START + 1)

// External RAM (8 KiB, switchable via cartridge mapper)
#define MMU_ADDRESS_EXTERNAL_RAM_START 		(0xA000)
#define MMU_ADDRESS_EXTERNAL_RAM_END 		(0xBFFF)
#define MMU_EXTERNAL_RAM_SIZE				(MMU_ADDRESS_EXTERNAL_RAM_END - MMU_ADDRESS_EXTERNAL_RAM_START + 1)

// Work RAM Bank A (WRAM 0) (4 KiB)
#define MMU_ADDRESS_WORK_RAM_A_START 		(0xC000)
#define MMU_ADDRESS_WORK_RAM_A_END			(0xCFFF)
#define MMU_WORK_RAM_A_SIZE					(MMU_ADDRESS_WORK_RAM_A_END - MMU_ADDRESS_WORK_RAM_A_START + 1)

// Work RAM Bank B (WRAM 1–7 in CGB) (4 KiB)
#define MMU_ADDRESS_WORK_RAM_B_START		(0xD000)
#define MMU_ADDRESS_WORK_RAM_B_END 			(0xDFFF)
#define MMU_WORK_RAM_B_SIZE					(MMU_ADDRESS_WORK_RAM_B_END - MMU_ADDRESS_WORK_RAM_B_START + 1)

// Combined WRAM region end address
#define MMU_ADDRESS_WORK_RAM_END			(0xDFFF)

// Echo RAM (0xE000–0xFDFF) (Mirror of WRAM C000–DDFF)
#define MMU_ADDRESS_ECHO_RAM_START			(0xE000)
#define MMU_ADDRESS_ECHO_RAM_END 			(0xFDFF)

// Object Attribute Memory (OAM) (0xFE00–0xFE9F)
#define MMU_ADDRESS_OAM_START				(0xFE00)
#define MMU_ADDRESS_OAM_END 				(0xFE9F)
#define MMU_OAM_SIZE						(MMU_ADDRESS_OAM_END - MMU_ADDRESS_OAM_START + 1)

// Not Usable Area (0xFEA0–0xFEFF)
#define MMU_ADDRESS_NOT_USABLE_START		(0xFEA0)
#define MMU_ADDRESS_NOT_USABLE_END			(0xFEFF)
#define MMU_NOT_USABLE_SIZE					(MMU_ADDRESS_NOT_USABLE_END - MMU_ADDRESS_NOT_USABLE_START + 1)

// I/O Registers (0xFF00–0xFF7F)
#define MMU_ADDRESS_I_O_REGISTER_START		(0xFF00)
#define MMU_ADDRESS_I_O_REGISTER_END		(0xFF7F)
#define MMU_I_O_REGISTER_SIZE				(MMU_ADDRESS_I_O_REGISTER_END - MMU_ADDRESS_I_O_REGISTER_START + 1)

// High RAM (HRAM) (0xFF80–0xFFFE)
#define MMU_ADDRESS_HIGH_RAM_START 			(0xFF80)
#define MMU_ADDRESS_HIGH_RAM_END 			(0xFFFE)
#define MMU_HIGH_RAM_SIZE					(MMU_ADDRESS_HIGH_RAM_END - MMU_ADDRESS_HIGH_RAM_START + 1)

// Interrupt Enable Register (IE) (0xFFFF)
#define MMU_ADDRESS_INTERRUPT_ENABLE_REGISTER		(0xFFFF)

// ----------------------------------------------------------------------
// External Memory Array Declarations
// These arrays are defined in mmu.c and declared here as extern
// so other components can access them (e.g., the CPU or PPU).
// ----------------------------------------------------------------------
extern uint8_t rom_bank_00[MMU_ROM_BANK_00_SIZE];
extern uint8_t rom_bank_01[MMU_ROM_BANK_01_SIZE];
extern uint8_t v_ram[MMU_V_RAM_SIZE];
extern uint8_t external_ram[MMU_EXTERNAL_RAM_SIZE];
extern uint8_t work_ram_a[MMU_WORK_RAM_A_SIZE];
extern uint8_t work_ram_b[MMU_WORK_RAM_B_SIZE];
extern uint8_t oam[MMU_OAM_SIZE];
extern uint8_t not_usable[MMU_NOT_USABLE_SIZE];
extern uint8_t i_o_register[MMU_I_O_REGISTER_SIZE];
extern uint8_t high_ram[MMU_HIGH_RAM_SIZE];
extern uint8_t interrupt_enable;

// ----------------------------------------------------------------------
// MMU Access Function Prototypes
// ----------------------------------------------------------------------
uint8_t mmu_read_byte(uint16_t address);
void mmu_write_byte(uint16_t address, uint8_t value);

// Function to load the ROM file into memory
void mmu_load_rom(const char* filename);


#endif /* COMPONENTS_MMU_H_ */
