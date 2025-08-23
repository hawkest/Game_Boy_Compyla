/*
 * ppu.h
 *
 *  Created on: 10 Aug 2025
 *      Author: hawke
 */

#ifndef COMPONENTS_PPU_H_
#define COMPONENTS_PPU_H_

#include <stdint.h>
#include "..\headers\mystdbool.h"
#include "..\BitOps\bit_macros.h"

// Default Power-On Values for PPU Registers
#define PPU_DEFAULT_LCDC_VALUE  (0x91)
#define PPU_DEFAULT_STAT_VALUE  (0x02)
#define PPU_DEFAULT_SCY_VALUE   (0x00)
#define PPU_DEFAULT_SCX_VALUE   (0x00)
#define PPU_DEFAULT_LY_VALUE    (0x00)
#define PPU_DEFAULT_LYC_VALUE   (0x00)
#define PPU_DEFAULT_BGP_VALUE   (0xFC)
#define PPU_DEFAULT_OBP0_VALUE  (0xFF)
#define PPU_DEFAULT_OBP1_VALUE  (0xFF)
#define PPU_DEFAULT_WY_VALUE    (0x00)
#define PPU_DEFAULT_WX_VALUE    (0x00)

// And the pixel dimensions for your screen_buffer
#define GB_SCREEN_WIDTH   (160)
#define GB_SCREEN_HEIGHT  (144)


// LCDC 0XFF40 BYTE MAP
#define PPU_LCDC_LCD_PPU_ENABLE 			BIT(7)
#define PPU_LCDC_WINDOW_TILE_MAP_SELECT		BIT(6)
#define PPU_LCDC_WINDOW_DISPLAY_ENABLE		BIT(5)
#define PPU_LCDC_BG_WINDOW_TILE_SELECT		BIT(4)
#define PPU_LCDC_BG_TILE_MAP_DISPLAY_SELECT	BIT(3)
#define PPU_LCDC_OBJ_SPRITE_SIZE			BIT(2)
#define PPU_LCDC_OBJ_SPRITE_DISPLAY_ENABLE 	BIT(1)
#define PPU_LCDC_BG_DISPLAY_PRIORITY 		BIT(0)

// STAT 0XFF41 BYTE MAP
//#define PPU_STAT_							BIT(7)
#define PPU_STAT_LYC_LC_INTERRUPT_ENABLE			BIT(6)
#define PPU_STAT_MODE_2_OAM_INTERRUPT_ENABLE 		BIT(5)
#define PPU_STAT_MODE_1_VBLANK_INTERRUPT_ENABLE		BIT(4)
#define PPU_STAT_MODE_0_HBLANK_INTERRUPT_ENABLE		BIT(3)
#define PPU_STAT_LYC_LC_FLAG						BIT(2)
#define PPU_STAT_MODE_FLAG_BIT_1 					BIT(1)
#define PPU_STAT_MODE_FLAG_BIT_0 					BIT(0)

typedef enum
{
    PPU_MODE_HBLANK = 0,    // Mode 0
    PPU_MODE_VBLANK = 1,    // Mode 1
    PPU_MODE_OAM_SCAN = 2,  // Mode 2
    PPU_MODE_DRAWING = 3    // Mode 3
} ppu_mode_t;

typedef struct
{
	ppu_mode_t current_mode; 		// What mode the PPU is currently in (H-Blank, V-Blank, etc.)
	uint32_t cycles_on_scanline; 	// Counter for CPU cycles spent on the current scanline
	uint8_t internal_ly_counter; 	// PPU's internal counter for the current scanline (LY register value)
	uint8_t current_lyc_value;
	myBool lcd_enabled;

    // Decoded palettes for faster lookups during rendering
	uint32_t bg_palette[4];			// Decoded colors for background/window
	uint32_t obj_palette_0[4];		// Decoded colors for sprite palette 0
	uint32_t obj_palette_1[4];		// Decoded colors for sprite palette 1

    // DMA transfer state
    myBool dma_active;				// True if an OAM DMA transfer is currently in progress
    uint16_t dma_cycles_left;		// Cycles remaining for the current DMA transfer

    // Pixel buffers
	uint32_t screen_buffer[GB_SCREEN_WIDTH * GB_SCREEN_HEIGHT];	// The full screen frame buffer
    uint32_t scanline_pixels[GB_SCREEN_WIDTH];					// Temporary buffer for the current scanline being rendered
} ppu_state_t;

extern ppu_state_t ppu_state;
void ppu_init(void);
void ppu_step(uint32_t cpu_cycles_executed_this_turn);

#endif /* COMPONENTS_PPU_H_ */
