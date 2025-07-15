/*
 * bit_macros.h
 *
 *  Created on: 15 Jul 2025
 *      Author: hawke
 */

#ifndef BITOPS_BIT_MACROS_H_
#define BITOPS_BIT_MACROS_H_

#include <stdint.h> // For uint8_t, uint16_t, uint32_t, etc.

// --- Basic Bit Manipulation ---

/**
 * @brief Creates a bitmask for a single bit at a given position.
 * Uses uint32_t for the shift base to prevent "shift >= width of type" warnings
 * if BIT_POS approaches the size of 'int'.
 * @param BIT_POS The 0-indexed position of the bit (e.g., 0 for LSB, 7 for MSB of a byte).
 */
#define BIT(BIT_POS) ( (uint32_t)1U << (BIT_POS) )

/**
 * @brief Checks if a specific bit is set in a value.
 * @param VALUE The value to check.
 * @param BIT_POS The 0-indexed position of the bit.
 * @return True (non-zero) if the bit is set, False (zero) otherwise.
 */
#define CHK_BIT(VALUE, BIT_POS) ((VALUE) & BIT(BIT_POS))

/**
 * @brief Sets a specific bit in a value.
 * @param VALUE The value to modify (L-value).
 * @param BIT_POS The 0-indexed position of the bit to set.
 */
#define SET_BIT(VALUE, BIT_POS) ((VALUE) |= BIT(BIT_POS))

/**
 * @brief Clears a specific bit in a value.
 * @param VALUE The value to modify (L-value).
 * @param BIT_POS The 0-indexed position of the bit to clear.
 */
#define CLR_BIT(VALUE, BIT_POS) ((VALUE) &= ~BIT(BIT_POS))

/**
 * @brief Toggles (flips) a specific bit in a value.
 * @param VALUE The value to modify (L-value).
 * @param BIT_POS The 0-indexed position of the bit to toggle.
 */
#define TOGGLE_BIT(VALUE, BIT_POS) ((VALUE) ^= BIT(BIT_POS))


// --- Bitfield Extraction and Insertion ---

/**
 * @brief Creates a mask for a bitfield.
 * Handles cases where NUM_BITS might be 32 (or more) to prevent shift warnings.
 * @param NUM_BITS The number of bits in the field.
 * @return A mask with NUM_BITS set, right-aligned.
 */
#define CREATE_BITFIELD_MASK(NUM_BITS) \
    ( (NUM_BITS) >= 32 ? 0xFFFFFFFFUL : ( (uint32_t)1U << (NUM_BITS) ) - 1U )


/**
 * @brief Extracts a bitfield from a value.
 * @param VALUE The source value.
 * @param START_BIT The 0-indexed starting bit position of the field (inclusive).
 * @param NUM_BITS The number of bits in the field.
 * @return The extracted bitfield value, right-aligned.
 */
#define GET_BITFIELD(VALUE, START_BIT, NUM_BITS) \
    (((VALUE) >> (START_BIT)) & CREATE_BITFIELD_MASK(NUM_BITS))

/**
 * @brief Inserts a value into a bitfield within a larger value.
 * @param TARGET_VALUE The value to modify (L-value) where the bitfield will be inserted.
 * @param START_BIT The 0-indexed starting bit position of the field (inclusive).
 * @param NUM_BITS The number of bits in the field.
 * @param INSERT_VALUE The value to insert into the bitfield.
 */
#define SET_BITFIELD(TARGET_VALUE, START_BIT, NUM_BITS, INSERT_VALUE) \
    do { \
        uint32_t mask_val = CREATE_BITFIELD_MASK(NUM_BITS); \
        uint32_t mask = mask_val << (START_BIT); \
        (TARGET_VALUE) = ((TARGET_VALUE) & ~mask) | (((INSERT_VALUE) << (START_BIT)) & mask); \
    } while(0)

#endif /* BITOPS_BIT_MACROS_H_ */
