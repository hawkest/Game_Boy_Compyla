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


ppu_state_t ppu_state;

void ppu_init(void)
{
	ppu_state.current_mode = PPU_MODE_OAM_SCAN;
	ppu_state.cycles_on_scanline = 0;
	memset(ppu_state.screen_buffer, 0, 160 * 144 * 4); // array is 160*144*uint32_t so 160*144*4bytes

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

}

