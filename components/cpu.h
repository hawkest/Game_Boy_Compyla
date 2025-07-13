/*
 * cpu.h
 *
 * Defines the structure for the Game Boy CPU registers (Sharp LR35902).
 * Uses anonymous unions and structs for efficient access to 8-bit and 16-bit registers.
 * Includes definitions for CPU flags and declares the global CPU state structure.
 */



#ifndef COMPONENTS_CPU_H_
#define COMPONENTS_CPU_H_

#include <stdint.h>

// ----------------------------------------------------------------------
// CPU Flag Definitions
// These constants define the bit positions for the CPU's Flag Register (F).
// The Flag Register is part of the AF pair.
// ----------------------------------------------------------------------
#define CPU_FLAG_ZERO_Z_BIT		(1 << 7)	// 0x80 (Set when an operation results in zero)
#define CPU_FLAG_SUB_N_BIT		(1 << 6)	// 0x40 (Set if the last instruction was a subtraction)
#define CPU_FLAG_HALF_H_BIT		(1 << 5)	// 0x20 (Set if there was a carry/borrow in the lower nibble)
#define CPU_FLAG_CARRY_C_BIT	(1 << 4)	// 0x10 (Set if there was a carry/borrow in the high byte)


// ----------------------------------------------------------------------
// CPU_State Structure
// Defines the CPU registers and their relationships using anonymous unions.
// ----------------------------------------------------------------------
typedef struct {

    // 16-bit registers: Program Counter (PC) and Stack Pointer (SP)
    uint16_t PC; // Program Counter: Points to the next instruction to be executed
    uint16_t SP; // Stack Pointer: Points to the current stack top

    // Register pairs defined using unions and structs.
    // The Game Boy CPU is based on the Z80, which is little-endian.
    // We define the structure members (LSB and MSB) to match the expected
    // layout in memory for correct 16-bit access (e.g., in BC, C is the LSB, B is the MSB).

    // AF Register Pair (A: Accumulator, F: Flags)
    union {
        uint16_t AF; // 16-bit access to AF
        struct {
            uint8_t F; // Flags register (LSB in little-endian struct)
            uint8_t A; // Accumulator (MSB in little-endian struct)
        };
    };

    // BC Register Pair
    union {
        uint16_t BC; // 16-bit access to BC
        struct {
            uint8_t C; // LSB
            uint8_t B; // MSB
        };
    };

    // DE Register Pair
    union {
        uint16_t DE; // 16-bit access to DE
        struct {
            uint8_t E; // LSB
            uint8_t D; // MSB
        };
    };

    // HL Register Pair (Used for addressing and data manipulation)
    union {
        uint16_t HL; // 16-bit access to HL
        struct {
            uint8_t L; // LSB
            uint8_t H; // MSB
        };
    };

} CPU_State;

// ----------------------------------------------------------------------
// Global CPU State Declaration
// The actual instance of the CPU state (cpu_regs) is defined in cpu.c
// and declared here as 'extern' so other components can access it.
// ----------------------------------------------------------------------
extern CPU_State cpu_regs;
extern void cpu_init();
extern void cpu_run();

#endif /* COMPONENTS_CPU_H_ */
