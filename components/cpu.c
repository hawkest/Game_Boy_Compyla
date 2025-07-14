/*
 * cpu.c
 *
 *  Created on: 12 Jul 2025
 *      Author: hawke
 */


#include "cpu.h"
#include "mmu.h"

CPU_State cpu_regs;

void cpu_init();
void cpu_run();
static void cpu_step();
static void cpu_execute(uint8_t opcode);

/*HELPER FUNCTIONS*/
static uint16_t read_imm16();
static void write_imm16(uint16_t address, uint16_t value);

#define true (1)
#define false (0)

int running = true;


void cpu_init()
{
	cpu_regs.PC = 0x0100;
	cpu_regs.SP = 0xFFFE;
	cpu_regs.AF = 0x01B0;
	cpu_regs.BC = 0x0013;
	cpu_regs.DE = 0x00D8;
	cpu_regs.HL = 0x014D;

}

void cpu_run()
{
	while(running)
	{
		cpu_step();
	}
}

static void cpu_step()
{
	uint8_t opcode;
	opcode = mmu_read_byte(cpu_regs.PC);
	cpu_regs.PC = cpu_regs.PC + 1;

	cpu_execute(opcode);
}

static void cpu_execute(uint8_t opcode)
{
	switch (opcode)
	{
		case 0x00: // NOP
		{
			// No operation needed
		}
		break;

		case 0x01: // ld BC, imm16
		{
			cpu_regs.BC = read_imm16();
		}
		break;

		case 0x11: // ld DE, imm16
		{
			cpu_regs.DE = read_imm16();
		}
		break;

		case 0x21: // ld HL, imm16
		{
			cpu_regs.HL = read_imm16();
		}
		break;

		case 0x31: // ld SP, imm16
		{
			cpu_regs.SP = read_imm16();
		}
		break;


		case 0x02: // ld (BC), A
		{
			mmu_write_byte(cpu_regs.BC, cpu_regs.A);
		}
		break;

		case 0x12: // ld (DE), A
		{
			mmu_write_byte(cpu_regs.DE, cpu_regs.A);
		}
		break;

		case 0x22: // ld (HL+), A
		{
			mmu_write_byte(cpu_regs.HL++, cpu_regs.A);
		}
		break;

		case 0x32: // ld (HL-), A
		{
			mmu_write_byte(cpu_regs.HL--, cpu_regs.A);
		}
		break;



		case 0x0A: //ld a, (BC)
		{
			cpu_regs.A = mmu_read_byte(cpu_regs.BC);
		}
		break;

		case 0x1A: //ld a, (DE)
		{
			cpu_regs.A = mmu_read_byte(cpu_regs.DE);
		}
		break;

		case 0x2A: //ld a, (HL+)
		{
			cpu_regs.A = mmu_read_byte(cpu_regs.HL++);
		}
		break;

		case 0x3A: //ld a, (HL-)
		{
			cpu_regs.A = mmu_read_byte(cpu_regs.HL--);
		}
		break;


		case 0x08: // LD (imm16), SP
		{
			uint16_t target_address = read_imm16();
			write_imm16(target_address, cpu_regs.SP);
		}
		break;


		case 0x03: //increment BC
		{
			cpu_regs.BC++;
		}
		break;

		case 0x13: //increment DE
		{
			cpu_regs.DE++;
		}
		break;

		case 0x23: //increment HL
		{
			cpu_regs.HL++;
		}
		break;

		case 0x33: //increment SP
		{
			cpu_regs.SP++;
		}
		break;


		case 0x0B: //decrement BC
		{
			cpu_regs.BC--;
		}
		break;

		case 0x1B: //decrement DE
		{
			cpu_regs.DE--;
		}
		break;

		case 0x2B: //decrement HL
		{
			cpu_regs.HL--;
		}
		break;

		case 0x3B: //decrement SP
		{
			cpu_regs.SP--;
		}
		break;



		case 0x09: //add hl, BC
		{
			cpu_regs.HL += cpu_regs.BC;
		}
		break;

		case 0x19: //add hl, DE
		{
			cpu_regs.HL += cpu_regs.DE;
		}
		break;

		case 0x29: //add hl, HL
		{
			cpu_regs.HL += cpu_regs.HL;
		}
		break;

		case 0x39: //add hl, SP
		{
			cpu_regs.HL += cpu_regs.SP;
		}
		break;




		default:
			// Handle unknown/unimplemented opcodes (e.g., print an error)
			break;
	}
}

static uint16_t read_imm16()
{
    uint8_t lsb = mmu_read_byte(cpu_regs.PC);
    cpu_regs.PC++; // Increment PC after reading LSB
    uint8_t msb = mmu_read_byte(cpu_regs.PC);
    cpu_regs.PC++; // Increment PC after reading MSB
    return lsb | (msb << 8); // Combine bytes (little-endian)
}

static void write_imm16(uint16_t address, uint16_t value)
{
    // Write the Low Byte of SP to target_address
    mmu_write_byte(address, (uint8_t)(value & 0x00FF));

    // Write the High Byte of SP to target_address + 1
    mmu_write_byte(address + 1, (uint8_t)((value >> 8) & 0x00FF));
}


