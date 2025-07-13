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

		case 0x01: // ld r16, imm16
		{
			uint16_t imm16;
			uint8_t LSB, MSB;
			LSB = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC = cpu_regs.PC + 1;
			MSB = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC = cpu_regs.PC + 1;
			imm16 = LSB | (MSB << 8);

			cpu_regs.BC = imm16;
		}
		break;

		case 0x02: // ld r16mem, a
		{
			mmu_write_byte(cpu_regs.DE, cpu_regs.A);
		}
		break;

		default:
			// Handle unknown/unimplemented opcodes (e.g., print an error)
			break;
	}
}
