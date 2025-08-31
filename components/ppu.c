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

//void ppu_decode_palette(uint8_t palette_data_register_value, uint32_t *target_palette_array);
void ppu_render_scanline(void);
void render_background_layer_for(uint8_t current_scanline_y);
void render_window_layer_for(uint8_t current_scanline_y);
void render_sprite_layer_for(uint8_t current_scanline_y);

// Note: Colours are defined in 0xRRGGBBAA format (Red, Green, Blue, Alpha).
// You can adjust the alpha (A) value if your rendering library requires it.
// Here, we use FF for full opacity.

// --- 1. Classic Green palette (Original Game Boy) ---
// This palette maps to the original green-on-dark-green screen.
const uint32_t CLASSIC_GREEN_PALETTE[4] = {
    0xFF9BBC0F, // White (Lightest shade)
    0xFF8BAC0F, // Light Grey
    0xFF306230, // Dark Grey
    0xFF0F380F  // Black (Darkest shade)
};

// --- 2. Classic Grayscale Palette ---
// A clean and simple black and white palette.
const uint32_t CLASSIC_GRAYSCALE_PALETTE[4] = {
    0xFFFFFFFF, // White (Lightest shade)
    0xFFC0C0C0, // Light Grey
    0xFF606060, // Dark Grey
    0xFF000000  // Black (Darkest shade)
};

// --- 3. Modern Vibrant Palette ---
// A more colorful and high-contrast palette.
const uint32_t MODERN_VIBRANT_PALETTE[4] = {
    0xFFF6F89B, // Lightest shade
    0xFF87C042, // Light shade
    0xFF2D6930, // Dark shade
    0xFF000000  // Darkest shade
};

// --- 4. Modern Purple Palette ---
// A clean, high-contrast purple palette.
const uint32_t MODERN_PURPLE_PALETTE[4] = {
    0xFFE0B0FF, // Lightest purple
    0xFF800080, // Medium purple
    0xFF48325C, // Dark purple
    0xFF200020  // Deepest purple
};


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
				ppu_render_scanline();
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

void ppu_render_scanline(void)
{
    // The current scanline we are rendering. This value comes from ppu_state.
    uint8_t current_scanline_y = ppu_state.internal_ly_counter;

    // Check the LCDC register to see what we need to render.
    uint8_t lcdc_register = mmu_read_byte(PPU_REGISTER_LCDC_ADDRESS);

    // Check if BG and Window layers are enabled.
    if (lcdc_register & PPU_LCDC_BG_DISPLAY_PRIORITY)
    {
        // Render the Background Layer (the base layer)
        // This is always drawn first and can be overwritten by other layers.
    	render_background_layer_for(current_scanline_y);

        // Check if the Window layer is enabled.
        if (lcdc_register & PPU_LCDC_WINDOW_DISPLAY_ENABLE)
		{
        	// Check if the Window's Y position has been reached.
            uint8_t window_y_pos = mmu_read_byte(PPU_REGISTER_WY_ADDRESS);
            if (current_scanline_y >= window_y_pos)
            {
            	// Render the Window Layer, which can overlap the background.
				render_window_layer_for(current_scanline_y);
            }
    	}
    }

    // Check if Sprites are enabled.
    if(lcdc_register & PPU_LCDC_OBJ_SPRITE_DISPLAY_ENABLE)
    {
    	// Render the Sprite Layer. Sprites are drawn on top of the BG and Window.
    	render_sprite_layer_for(current_scanline_y);
    }

    // After all the layers have been drawn for this scanline,
    // copy the final pixels to the main screen buffer.
    memcpy(ppu_state.screen_buffer + (current_scanline_y * GB_SCREEN_WIDTH), ppu_state.scanline_pixels , 4 * GB_SCREEN_WIDTH);

}

void render_background_layer_for(uint8_t current_scanline_y)
{
    // Get the current scroll positions from the MMU
    uint8_t scroll_x = mmu_read_byte(PPU_REGISTER_SCX_ADDRESS);
    uint8_t scroll_y = mmu_read_byte(PPU_REGISTER_SCY_ADDRESS);

    // Calculate the 'y' position on the 256x256 pixel background map
    // The modulo operation ensures we wrap around the map if scrolling goes past the edge.
    uint8_t background_map_y = (current_scanline_y + scroll_y) % 256;

    // Get the base address of the background map data from LCDC register
    uint8_t lcdc_register = mmu_read_byte(PPU_REGISTER_LCDC_ADDRESS);

    uint16_t background_map_address, tile_data_address = 0xffff;

    if(lcdc_register & PPU_LCDC_BG_TILE_MAP_DISPLAY_SELECT)
    {
    	background_map_address = 0x9C00;
    }
    else
    {
    	background_map_address = 0x9800;
    }

    // Get the base address of the tile pattern data
    if (lcdc_register & PPU_LCDC_BG_WINDOW_TILE_SELECT)
	{
    	tile_data_address = 0x8000;
	}
    else
    {
        tile_data_address = 0x8800;
    }

    // Loop through each pixel on the current scanline (0 to 159)
    for (int p_x = 0; p_x <= 159; p_x++)
    {
        // Calculate the 'x' position on the background map
        uint16_t background_map_x = (p_x + scroll_x) % 256;

        // Determine which 8x8 tile we need to look at
        // Divide by 8 to convert pixel coords to tile coords
        uint8_t tile_x = background_map_x / 8;
        uint8_t tile_y = background_map_y / 8;

        // Find the index of the tile in the background map
        // The background map is 32 tiles wide.
        uint8_t tile_index = mmu_read_byte(background_map_address + (tile_y * 32) + tile_x);

        // Find the 2-bit color ID of the specific pixel within that tile
        // Remember that each row of an 8x8 tile is represented by 2 bytes of data.
        uint16_t tile_row = background_map_y % 8;
        uint16_t tile_column = background_map_x % 8;

        // Use the tile_index to read the two bytes that define the row of pixels
        // The address of the tile's data is calculated using the base tile_data_address + a specific offset
        // This offset changes based on the tile_data_address, so you will need to handle that logic
        // This is a subtle point that you will need to implement, i have left it vague for you to solve
        uint16_t tile_start_address = tile_data_address + (tile_index * 16);

        // Use bitwise logic to get the 2-bit color ID from those two bytes
        uint8_t byte_1, byte_2;
        byte_1 = mmu_read_byte(tile_start_address + (tile_row * 2));
        byte_2 = mmu_read_byte(tile_start_address + (tile_row * 2) + 1);

        uint8_t low_bit = (byte_1 >> (7 - tile_column )) & 1
        uint8_t high_bit = (byte_2 >> (7 - tile column)) & 1;

        // Use the bg_palette to translate the 2-bit ID into a 32-bit RGBA color
        // The palette was already set up by your `ppu_decode_palette` function.
        uint32_t final_colour = ppu_state.bg_palette[colour_id];

        // Store the final color in the `scanline_pixels` array.
        ppu_state.scanline_pixels[p_x] = final_colour;
    }
//END FUNCTION
}

void render_window_layer_for(uint8_t current_scanline_y)
{//FUNCTION render_window_layer_for(current_scanline_y):
//    // Get the current window position from the MMU
//    window_x_pos = mmu_read_byte(PPU_REGISTER_WX_ADDRESS)
//    window_y_pos = mmu_read_byte(PPU_REGISTER_WY_ADDRESS)
//
//    // The Window's Y position is a simple check. We already did this check
//    // in the main render function, so we don't need to do it again here.
//
//    // Determine the base address of the Window's tile map
//    lcdc_register = mmu_read_byte(PPU_REGISTER_LCDC_ADDRESS)
//    IF (lcdc_register BITWISE AND PPU_LCDC_WINDOW_TILE_MAP_DISPLAY_SELECT_FLAG):
//        window_map_address = 0x9C00
//    ELSE:
//        window_map_address = 0x9800
//    END IF
//
//    // Get the base address of the tile pattern data
//    IF (lcdc_register BITWISE AND PPU_LCDC_BG_WINDOW_TILE_SELECT_FLAG):
//        tile_data_address = 0x8000
//    ELSE:
//        tile_data_address = 0x8800
//    END IF
//
//    // The Game Boy's Window has a small quirk: WX is offset by 7.
//    // A value of 7 means the Window starts at X position 0.
//    start_x_on_screen = window_x_pos - 7
//
//    // Track the pixel 'y' within the window itself, starting from 0.
//    window_y_offset = current_scanline_y - window_y_pos
//
//    // Loop through each pixel on the scanline, but only draw inside the window's area.
//    FOR pixel_x FROM start_x_on_screen TO 159:
//        // Calculate the 'x' position within the window, starting from 0.
//        window_x_offset = pixel_x - start_x_on_screen
//
//        // Determine the tile coordinates within the window map.
//        // Divide by 8 to convert pixel coords to tile coords.
//        tile_x = window_x_offset / 8
//        tile_y = window_y_offset / 8
//
//        // Find the index of the tile in the window map.
//        tile_index = mmu_read_byte(window_map_address + (tile_y * 32) + tile_x)
//
//        // Find the 2-bit color ID of the specific pixel within that tile.
//        // This logic is the same as the background layer.
//        tile_row = window_y_offset MODULO 8
//        tile_column = window_x_offset MODULO 8
//
//        // Use the tile_index to read the two bytes that define the row of pixels
//        // The address of the tile's data is calculated using the base tile_data_address + a specific offset
//        // This is a subtle point that you will need to implement, i have left it vague for you to solve
//
//        // Use bitwise logic to get the 2-bit color ID from those two bytes.
//
//        // Use the bg_palette to translate the 2-bit ID into a 32-bit RGBA color.
//        final_color = ppu_state.bg_palette[color_id]
//
//        // Overwrite the background pixels at this position with the window's pixel color.
//        ppu_state.scanline_pixels[pixel_x] = final_color
//
//    END FOR
//END FUNCTION
}

void render_sprite_layer_for(uint8_t current_scanline_y)
{
//FUNCTION render_sprite_layer_for(current_scanline_y):
//	// Get the sprite height (8x8 or 8x16) from the LCDC register.
//	lcdc_register = mmu_read_byte(PPU_REGISTER_LCDC_ADDRESS)
//	sprite_height = (lcdc_register BITWISE AND PPU_LCDC_OBJ_SPRITE_SIZE) ? 16 : 8
//
//	// There are 40 possible sprites, stored in OAM from 0xFE00 to 0xFE9F.
//	// Each sprite is 4 bytes of data.
//	FOR sprite_index FROM 0 TO 39:
//		// Get the sprite's 4 bytes of data from OAM.
//		sprite_data_address = 0xFE00 + (sprite_index * 4)
//		sprite_y_pos = mmu_read_byte(sprite_data_address + 0)
//		sprite_x_pos = mmu_read_byte(sprite_data_address + 1)
//		tile_index = mmu_read_byte(sprite_data_address + 2)
//		sprite_flags = mmu_read_byte(sprite_data_address + 3)
//
//		// Check if the sprite is on the current scanline.
//		// Sprites are positioned at Y-16 on the screen.
//		IF (current_scanline_y >= (sprite_y_pos - 16) AND current_scanline_y < (sprite_y_pos - 16 + sprite_height)):
//			// This sprite is visible on the current line. Now we render its pixels.
//
//			// Get the tile data from VRAM. Sprites always use the 0x8000 tile data address space.
//			tile_data_address = 0x8000
//
//			// Apply vertical flip logic.
//			// This is a subtle point. If the sprite is flipped vertically, you need to
//			// access its tile data in reverse. For example, if a sprite is 8 pixels tall,
//			// pixel row 0 on the screen corresponds to row 7 in the tile data.
//			// You will need to use a bitwise AND on the sprite_flags and check a specific bit.
//
//			// Loop through each pixel column of the sprite (0 to 7)
//			FOR sprite_pixel_x FROM 0 TO 7:
//				// Apply horizontal flip logic.
//				// This is a subtle point. If the sprite is flipped horizontally, you need to
//				// access its tile data from right-to-left.
//
//				// Get the 2-bit color ID of the pixel from the tile data.
//
//				// Check for transparency.
//				// Color ID 0 is always transparent for sprites. Do not draw this pixel.
//				IF color_id IS NOT 0:
//					// Use the sprite_flags to determine which palette to use (OBP0 or OBP1).
//					// This is a bitwise AND on the sprite_flags and a specific bit.
//
//					// Check for sprite priority.
//					// If the sprite has low priority, it can be drawn behind the background/window.
//					// This is also a bitwise AND on the sprite_flags and a specific bit.
//
//					// If the sprite has high priority, just draw it.
//					// If the sprite has low priority, check if the background pixel at this
//					// location is non-white (color ID 0). If it is, DO NOT draw the sprite.
//
//					// Use the correct palette to get the final RGBA color.
//					final_color = ppu_state.obj_palette_[0/1][color_id]
//
//					// Overwrite the pixel in the scanline_pixels array.
//					ppu_state.scanline_pixels[sprite_x_pos + sprite_pixel_x] = final_color
//				END IF
//			END FOR
//		END IF
//	END FOR
//END FUNCTION
}
