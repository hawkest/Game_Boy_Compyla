/*
 * ppu.c
 *
 *  Created on: 10 Aug 2025
 *      Author: hawke
 */

#include <string.h>

#include "ppu.h"
#include "mmu.h"
#include "..\headers\mystdbool.h"
#include "..\BitOps\bit_macros.h"


ppu_state_t ppu_state;

void ppu_decode_palette(uint8_t palette_data_register_value, uint32_t *target_palette_array);


void ppu_init(void)
{
	ppu_state.current_mode = PPU_MODE_OAM_SCAN;
	ppu_state.cycles_on_scanline = 0x00;
	ppu_state.internal_ly_counter = 0x00;
	ppu_state.lcd_enabled = myFalse;

	memset(ppu_state.bg_palette, 0x00 , 4 * 4);
	memset(ppu_state.obj_palette_0, 0 , 4 * 4);
	memset(ppu_state.obj_palette_1, 0 , 4 * 4);

	ppu_state.dma_active = myFalse;
	ppu_state.dma_cycles_left = 0x00;

	memset(ppu_state.screen_buffer, 0x00, GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT * 4); // array is 160*144*uint32_t so 160*144*4bytes
	memset(ppu_state.scanline_pixels, 0x00, GB_SCREEN_WIDTH * 4);



	mmu_write_byte(PPU_REGISTER_LCDC_ADDRESS, PPU_DEFAULT_LCDC_VALUE);
	mmu_write_byte(PPU_REGISTER_STAT_ADDRESS, PPU_DEFAULT_STAT_VALUE);
	mmu_write_byte(PPU_REGISTER_SCY_ADDRESS,PPU_DEFAULT_SCY_VALUE);
	mmu_write_byte(PPU_REGISTER_SCX_ADDRESS,PPU_DEFAULT_SCX_VALUE);
	mmu_write_byte(PPU_REGISTER_LY_ADDRESS,PPU_DEFAULT_LY_VALUE);
	mmu_write_byte(PPU_REGISTER_LYC_ADDRESS,PPU_DEFAULT_LYC_VALUE);
//	mmu_write_byte(PPU_REGISTER_DMA_ADDRESS,); // The DMA register (0xFF46) is special; writing to it causes an action, so we don't 'initialize' it this way.
	mmu_write_byte(PPU_REGISTER_BGP_ADDRESS,PPU_DEFAULT_BGP_VALUE);
	mmu_write_byte(PPU_REGISTER_OBP0_ADDRESS,PPU_DEFAULT_OBP0_VALUE);
	mmu_write_byte(PPU_REGISTER_OBP1_ADDRESS,PPU_DEFAULT_OBP1_VALUE);
	mmu_write_byte(PPU_REGISTER_WY_ADDRESS,PPU_DEFAULT_WY_VALUE);
	mmu_write_byte(PPU_REGISTER_WX_ADDRESS,PPU_DEFAULT_WX_VALUE);

	ppu_decode_palette(mmu_read_byte(PPU_REGISTER_BGP_ADDRESS), ppu_state.bg_palette);
	ppu_decode_palette(mmu_read_byte(PPU_REGISTER_OBP0_ADDRESS), ppu_state.obj_palette_0);
	ppu_decode_palette(mmu_read_byte(PPU_REGISTER_OBP1_ADDRESS), ppu_state.obj_palette_1);

}

void ppu_step(uint32_t cpu_cycles_executed_this_turn)
{

	if (ppu_state.lcd_enabled == myTrue)
	{

		// 1. Advance the PPU's Internal Clock:
		ppu_state.cycles_on_scanline += cpu_cycles_executed_this_turn;

		// 2. Manage PPU Mode Transitions:
		//    (The PPU cycles through modes based on how many cycles have passed on the current line.)

		if(ppu_state.current_mode == PPU_MODE_OAM_SCAN)//IF PPU's current_mode IS OAM_SCAN_MODE (Mode 2):
		{
			if(ppu_state.cycles_on_scanline >= 80)//IF PPU's internal_scanline_cycle_counter HAS REACHED APPROXIMATELY 80 CYCLES THEN
			{
				ppu_state.current_mode = PPU_MODE_DRAWING;//CHANGE PPU's current_mode TO DRAWING_MODE (Mode 3)
				// TODO (Future): Check if Mode 2 interrupts are enabled in STAT, and if so, trigger one.
			}
		}
		else if(ppu_state.current_mode == PPU_MODE_DRAWING)//ELSE IF PPU's current_mode IS DRAWING_MODE (Mode 3):
		{
			if(ppu_state.cycles_on_scanline >= 252)//IF PPU's internal_scanline_cycle_counter HAS REACHED APPROXIMATELY (80 + 172) CYCLES THEN // Total cycles for Mode 2 + Mode 3
			{
				ppu_state.current_mode = PPU_MODE_HBLANK; //CHANGE PPU's current_mode TO H_BLANK_MODE (Mode 0)
				// TODO (Future): Here's where the actual pixel drawing for the CURRENT_SCANLINE would happen.
				//                 It would fill a row in ppu_state.screen_buffer.
				// TODO (Future): Check if Mode 3 interrupts are enabled in STAT, and if so, trigger one.
			}
		}
		else if(ppu_state.current_mode == PPU_MODE_HBLANK)// ELSE IF PPU's current_mode IS H_BLANK_MODE (Mode 0)
		{
			if (ppu_state.cycles_on_scanline >= 456) //IF PPU's internal_scanline_cycle_counter HAS REACHED APPROXIMATELY 456 CYCLES THEN // Total cycles for a full visible scanline
			{
				ppu_state.internal_ly_counter += 1; //INCREMENT THE PPU's internal_LY_counter (scanline counter)
				mmu_write_byte(PPU_REGISTER_LY_ADDRESS, ppu_state.internal_ly_counter);
				ppu_state.cycles_on_scanline = 0; //RESET PPU's internal_scanline_cycle_counter TO ZERO

				if (ppu_state.internal_ly_counter < 144)	//IF internal_LY_counter IS LESS THAN 144 THEN // Still rendering visible lines (0-143)
				{
					ppu_state.current_mode = PPU_MODE_OAM_SCAN;// CHANGE PPU's current_mode TO OAM_SCAN_MODE (Mode 2) // Start the next scanline
						// TODO (Future): Check if Mode 0 interrupts are enabled in STAT, and if so, trigger one.
				}
				else // internal_LY_counter has reached 144, meaning V-Blank starts
				{
					ppu_state.current_mode = PPU_MODE_VBLANK; //CHANGE PPU's current_mode TO V_BLANK_MODE (Mode 1)
					// TODO (Future): Trigger a V-Blank Interrupt (set the V-Blank bit in the MMU's Interrupt Flag register).
					// TODO (Future): Check if Mode 1 interrupts are enabled in STAT, and if so, trigger one.
				}
			}
		}

		else if (ppu_state.current_mode == PPU_MODE_VBLANK)//ELSE IF PPU's current_mode IS V_BLANK_MODE (Mode 1):
		{
			if (ppu_state.cycles_on_scanline >= 456) //IF PPU's internal_scanline_cycle_counter HAS REACHED APPROXIMATELY 456 CYCLES THEN // Each V-Blank line also takes 456 cycles
			{
				ppu_state.internal_ly_counter += 1; //INCREMENT THE PPU's internal_LY_counter
				ppu_state.cycles_on_scanline = 0;//RESET PPU's internal_scanline_cycle_counter TO ZERO

				if (ppu_state.internal_ly_counter > 153) //IF internal_LY_counter IS GREATER THAN 153 THEN // End of V-Blank (LY reaches 154)
				{
					ppu_state.internal_ly_counter = 0; //RESET internal_LY_counter TO ZERO // Start a new frame, scanline counter back to 0
					ppu_state.current_mode = PPU_MODE_OAM_SCAN;//CHANGE PPU's current_mode TO OAM_SCAN_MODE (Mode 2) // Start rendering the first line of the new frame
				}
			}
		}

		// 3. Check for LY=LYC Match Condition:
		//    (This check should ideally happen frequently, or after LY increments)
		uint8_t current_LY_value = ppu_state.internal_ly_counter; //GET current_LY_value (from PPU's internal counter)
		uint8_t LYC_value = mmu_read_byte(PPU_REGISTER_LYC_ADDRESS); //GET LYC_value (from MMU by reading PPU_REGISTER_LYC_ADDRESS)

		// At the very end of ppu_step
		uint8_t current_stat_in_memory = mmu_read_byte(PPU_REGISTER_STAT_ADDRESS);

		// Preserve CPU-writable bits (interrupt enables)
		uint8_t preserved_cpu_bits = current_stat_in_memory & PPU_REGISTER_STAT_WRITABLE_MASK;

		// Get PPU-controlled mode bits
		uint8_t ppu_mode_bits = (uint8_t)ppu_state.current_mode; // Assuming PPU_MODE_... are 0, 1, 2, 3

		// Get LYC=LY flag bit
		uint8_t lyc_ly_flag = (current_LY_value == LYC_value) ? PPU_STAT_LYC_LC_FLAG : 0x00;

		// Combine all to form the new STAT byte
		uint8_t new_stat_value = preserved_cpu_bits | ppu_mode_bits | lyc_ly_flag;

		// Write the new STAT value back to memory
		mmu_write_byte(PPU_REGISTER_STAT_ADDRESS, new_stat_value);


	}
}

void ppu_decode_palette(uint8_t palette_data_register_value, uint32_t *target_palette_array)
{

    // A loop to process each of the four 2-bit color IDs in the byte.
    for (int i = 0; i <= 3; i++)
    {
        // Use a bitwise shift to move the correct 2-bit ID to the beginning (right side) of the byte.
        // For the first color (i=0), we shift 0 bits.
        // For the second color (i=1), we shift 2 bits.
        // For the third color (i=2), we shift 4 bits.
        // For the fourth color (i=3), we shift 6 bits.
        uint8_t shifted_byte = palette_data_register_value >> (i * 2);

        // Use a bitwise mask to isolate just the two bits at the end.
        // 0x03 in hexadecimal is 00000011 in binary.
        // This will give you a number from 0 to 3.
        uint8_t color_id = shifted_byte & 0x03;

        // Use the color_id to get the final color from your predefined palette.
        // Write the final color to your target array in the correct position.
        target_palette_array[i] = MODERN_PURPLE_PALETTE[color_id];

    }
}
