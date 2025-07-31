/*
 * mmu.c
 *
 * Implements the Memory Management Unit (MMU) for the Game Boy emulator.
 * Handles memory access routing for all addressable regions (ROM, RAM, VRAM, I/O).
 *
 * Created on: 12 Jul 2025
 * Author: hawke
 */

#include <stdint.h>
#include <stdio.h>
#include "mmu.h"

// ----------------------------------------------------------------------
// Global Memory Arrays
// These arrays represent the physical memory regions of the Game Boy.
// They correspond to the sizes defined in mmu.h.
// ----------------------------------------------------------------------
uint8_t rom_bank_00[MMU_ROM_BANK_00_SIZE];      // 0x0000 - 0x3FFF (16 KiB)
uint8_t rom_bank_01[MMU_ROM_BANK_01_SIZE];      // 0x4000 - 0x7FFF (16 KiB, switchable bank 1-NN)
uint8_t v_ram[MMU_V_RAM_SIZE];                  // 0x8000 - 0x9FFF (8 KiB Video RAM)
uint8_t external_ram[MMU_EXTERNAL_RAM_SIZE];    // 0xA000 - 0xBFFF (8 KiB External RAM)
uint8_t work_ram_a[MMU_WORK_RAM_A_SIZE];        // 0xC000 - 0xCFFF (4 KiB Work RAM Bank 0)
uint8_t work_ram_b[MMU_WORK_RAM_B_SIZE];        // 0xD000 - 0xDFFF (4 KiB Work RAM Bank 1)
uint8_t oam[MMU_OAM_SIZE];                      // 0xFE00 - 0xFE9F (Object Attribute Memory)
uint8_t not_usable[MMU_NOT_USABLE_SIZE];        // 0xFEA0 - 0xFEFF (Not Usable Area)
uint8_t i_o_register[MMU_I_O_REGISTER_SIZE];    // 0xFF00 - 0xFF7F (I/O Registers)
uint8_t high_ram[MMU_HIGH_RAM_SIZE];            // 0xFF80 - 0xFFFE (High RAM)
uint8_t interrupt_enable;                       // 0xFFFF (Interrupt Enable Register)


// ----------------------------------------------------------------------
// mmu_read_byte
// Reads a single byte from the specified 16-bit memory address.
// Routes the address to the correct memory region (ROM, RAM, VRAM, etc.).
// ----------------------------------------------------------------------
uint8_t mmu_read_byte(uint16_t address)
{
	// Initialize return_value to 0xFF, which is standard behavior for reads from
    // unmapped or invalid memory addresses on the Game Boy.
	uint8_t return_value = 0xFF;
	uint16_t offset = 0x0000;

	// Check if the address is the Interrupt Enable Register (0xFFFF)
	if (address == MMU_ADDRESS_INTERRUPT_ENABLE_REGISTER)
	{
		return_value = interrupt_enable;
	}
	// Check if the address is within the ROM region (0x0000 - 0x7FFF)
	else if (address <= MMU_ADDRESS_ROM_BANK_END)
	{
		// Check for ROM Bank 00 (0x0000 - 0x3FFF)
		if (address <= MMU_ADDRESS_ROM_BANK_00_END)
		{
			// Calculate offset relative to ROM Bank 00 start
			offset = address - MMU_ADDRESS_ROM_BANK_00_START;
			return_value = rom_bank_00[offset];
		}
		// If not Bank 00, it must be the switchable ROM Bank (0x4000 - 0x7FFF)
		else
		{
			// Calculate offset relative to ROM Bank 01 start
			offset = address - MMU_ADDRESS_ROM_BANK_01_NN_START;
			// Note: This implementation assumes Bank 01 is always selected for simplicity.
            // Proper emulation requires handling the switchable bank.
			return_value = rom_bank_01[offset];
		}
	}
	// Check for Video RAM (VRAM) (0x8000 - 0x9FFF)
	else if(address <= MMU_ADDRESS_V_RAM_END)
	{
		offset = address - MMU_ADDRESS_V_RAM_START;
		return_value = v_ram[offset];
	}
	// Check for External RAM (0xA000 - 0xBFFF)
	else if(address <= MMU_ADDRESS_EXTERNAL_RAM_END)
	{
		offset = address - MMU_ADDRESS_EXTERNAL_RAM_START;
		return_value = external_ram[offset];
	}
	// Check for Work RAM (WRAM A and B) (0xC000 - 0xDFFF)
	else if(address <= MMU_ADDRESS_WORK_RAM_END)
	{
		// Check for WRAM Bank A (0xC000 - 0xCFFF)
		if (address <= MMU_ADDRESS_WORK_RAM_A_END)
		{
			offset = address - MMU_ADDRESS_WORK_RAM_A_START;
			return_value = work_ram_a[offset];
		}
		// Check for WRAM Bank B (0xD000 - 0xDFFF)
		else
		{
			offset = address - MMU_ADDRESS_WORK_RAM_B_START;
			return_value = work_ram_b[offset];
		}

	}
	// Check for Echo RAM (0xE000 - 0xFDFF)
    // This is a mirror of WRAM (0xC000 - 0xDFFF).
    // The offset must map the E000 address back to the C000 range.
	else if(address <= MMU_ADDRESS_ECHO_RAM_END)
	{
		// The mirror offset is 0x2000 (0xE000 - 0xC000)
        uint16_t wram_mirrored_address = address - 0x2000;

        // We reuse the WRAM logic here to determine which WRAM bank to access (A or B)
        if (wram_mirrored_address <= MMU_ADDRESS_WORK_RAM_A_END)
		{
			offset = wram_mirrored_address - MMU_ADDRESS_WORK_RAM_A_START;
			return_value = work_ram_a[offset];
		}
		else
		{
			offset = wram_mirrored_address - MMU_ADDRESS_WORK_RAM_B_START;
			return_value = work_ram_b[offset];
		}
	}
	// Check for OAM (0xFE00 - 0xFE9F)
	else if(address <= MMU_ADDRESS_OAM_END)
	{
		offset = address - MMU_ADDRESS_OAM_START;
		return_value = oam[offset];
	}
	// Check for Not Usable Memory (0xFEA0 - 0xFEFF)
	else if(address <= MMU_ADDRESS_NOT_USABLE_END)
	{
        // Reads from this area often return 0xFF. If 'not_usable' is initialized to 0xFF,
        // the default return value handles this.
		offset = address - MMU_ADDRESS_NOT_USABLE_START;
		return_value = not_usable[offset];
	}
	// Check for I/O Registers (0xFF00 - 0xFF7F)
	else if(address <= MMU_ADDRESS_I_O_REGISTER_END)
	{
		offset = address - MMU_ADDRESS_I_O_REGISTER_START;
		return_value = i_o_register[offset];
	}
	// Check for High RAM (HRAM) (0xFF80 - 0xFFFE)
	else if(address <= MMU_ADDRESS_HIGH_RAM_END)
	{
		offset = address - MMU_ADDRESS_HIGH_RAM_START;
		return_value = high_ram[offset];
	}
    // If the address is not recognized by any of the above, return_value remains 0xFF (initialized at the start).

	return return_value;
}

// ----------------------------------------------------------------------
// mmu_write_byte
// Writes a single byte to the specified 16-bit memory address.
// ----------------------------------------------------------------------
void mmu_write_byte(uint16_t address, uint8_t value)
{
	uint16_t offset = 0x0000;

	// Handle Interrupt Enable Register (0xFFFF)
	if (address == MMU_ADDRESS_INTERRUPT_ENABLE_REGISTER)
	{
		interrupt_enable = value;
	}
//	// ROM (0x0000 - 0x7FFF)
//	// Writes to this region are typically ignored by the MMU (handled by MBCs if present).
//	else if (address <= MMU_ADDRESS_ROM_BANK_END)
//	{
//		if (address <= MMU_ADDRESS_ROM_BANK_00_END)
//		{
//			offset = address - MMU_ADDRESS_ROM_BANK_00_START;
//			// rom_bank_00[offset] = value; // Write ignored
//		}
//		else
//		{
//			offset = address - MMU_ADDRESS_ROM_BANK_01_NN_START;
//			// rom_bank_01[offset] = value; // Write ignored
//		}
//	}
	// Video RAM (VRAM) (0x8000 - 0x9FFF)
	else if(address <= MMU_ADDRESS_V_RAM_END)
	{
		offset = address - MMU_ADDRESS_V_RAM_START;
		v_ram[offset] = value;
	}
	// External RAM (0xA000 - 0xBFFF)
	else if(address <= MMU_ADDRESS_EXTERNAL_RAM_END)
	{
		offset = address - MMU_ADDRESS_EXTERNAL_RAM_START;
		external_ram[offset] = value;
	}
	// Work RAM (WRAM A and B) (0xC000 - 0xDFFF)
	else if(address <= MMU_ADDRESS_WORK_RAM_END)
	{
		// WRAM Bank A (0xC000 - 0xCFFF)
		if (address <= MMU_ADDRESS_WORK_RAM_A_END)
		{
			offset = address - MMU_ADDRESS_WORK_RAM_A_START;
			work_ram_a[offset] = value;
		}
		// WRAM Bank B (0xD000 - 0xDFFF)
		else
		{
			offset = address - MMU_ADDRESS_WORK_RAM_B_START;
			work_ram_b[offset] = value;
		}

	}
	// Echo RAM (0xE000 - 0xFDFF)
	// Writes to Echo RAM are mirrored to WRAM (0xC000 - 0xDFFF)
	else if(address <= MMU_ADDRESS_ECHO_RAM_END)
	{
		// Calculate the corresponding WRAM address by subtracting the mirror offset (0x2000)
		uint16_t wram_mirrored_address = address - 0x2000;

		// Route the write to the appropriate WRAM bank (A or B)
		if (wram_mirrored_address <= MMU_ADDRESS_WORK_RAM_A_END)
		{
			offset = wram_mirrored_address - MMU_ADDRESS_WORK_RAM_A_START;
			work_ram_a[offset] = value;
		}
		else
		{
			offset = wram_mirrored_address - MMU_ADDRESS_WORK_RAM_B_START;
			work_ram_b[offset] = value;
		}
	}
	// OAM (0xFE00 - 0xFE9F)
	else if(address <= MMU_ADDRESS_OAM_END)
	{
		offset = address - MMU_ADDRESS_OAM_START;
		oam[offset] = value;
	}
//	// Not Usable Memory (0xFEA0 - 0xFEFF)
//	// Writes to this region are ignored.
//	else if(address <= MMU_ADDRESS_NOT_USABLE_END)
//	{
//		offset = address - MMU_ADDRESS_NOT_USABLE_START;
//		// not_usable[offset] = value; // Write ignored
//	}
	// I/O Registers (0xFF00 - 0xFF7F)
	else if(address <= MMU_ADDRESS_I_O_REGISTER_END)
	{
		offset = address - MMU_ADDRESS_I_O_REGISTER_START;
		i_o_register[offset] = value;
	}
	// High RAM (HRAM) (0xFF80 - 0xFFFE)
	else if(address <= MMU_ADDRESS_HIGH_RAM_END)
	{
		offset = address - MMU_ADDRESS_HIGH_RAM_START;
		high_ram[offset] = value;
	}
    // Writes to other addresses (e.g., beyond 0xFFFF) are ignored implicitly.
}

void mmu_load_rom(const char* filename)
{
    FILE *file_ptr;
    // We open the file in binary read mode ("rb")
    file_ptr = fopen(filename, "rb");

    if (file_ptr == NULL)
    {
        printf("Error: Could not open ROM file: %s\n", filename);
        // In a full implementation, you might exit or return an error code.
        return;
    }

    // --- Loading ROM Bank 00 (0x0000 - 0x3FFF, first 16 KiB) ---
    // fread(destination, size_of_element, number_of_elements, file_pointer)
    // We read exactly MMU_ROM_BANK_00_SIZE bytes into rom_bank_00
    size_t bytes_read_bank_00 = fread(rom_bank_00, 1, MMU_ROM_BANK_00_SIZE, file_ptr);

    if (bytes_read_bank_00 != MMU_ROM_BANK_00_SIZE)
    {
        printf("Warning: Did not read full ROM Bank 00. Read %zu bytes.\n", bytes_read_bank_00);
        // For a basic ROM (no MBC), Bank 00 is crucial.
    }

    // --- Loading ROM Bank 01 (0x4000 - 0x7FFF, next 16 KiB) ---
    // We check if the ROM file is large enough to contain Bank 01 data
    size_t bytes_read_bank_01 = 0;

    // Check if the file still has data after reading Bank 00
    if (!feof(file_ptr))
    {
        // Read the next 16 KiB into rom_bank_01
        bytes_read_bank_01 = fread(rom_bank_01, 1, MMU_ROM_BANK_01_SIZE, file_ptr);
    }

    if (bytes_read_bank_01 < MMU_ROM_BANK_01_SIZE)
    {
        // This is normal for simple 32 KiB ROMs (Bank 01 is often switchable, but we load it here)
        // If the ROM is larger than 32 KiB, we'll need MBC handling later.
        printf("Note: Did not read full ROM Bank 01. Read %zu bytes.\n", bytes_read_bank_01);
    }

    // Close the file stream
    fclose(file_ptr);
}

uint16_t mmu_read_word(uint16_t address)
{
	    // Read the low byte from the current address
	    uint8_t low_byte = mmu_read_byte(address);

	    // Read the high byte from the next address
	    uint8_t high_byte = mmu_read_byte(address + 1);

	    // Combine them into a 16-bit word (little-endian)
	    return (high_byte << 8) | low_byte;
}

