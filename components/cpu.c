/*
 * cpu.c
 *
 *  Created on: 12 Jul 2025
 *      Author: hawke
 */


#include "cpu.h"
#include "mmu.h"
#include "..\BitOps\bit_macros.h"

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
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);

			uint16_t old_HL = cpu_regs.HL; // get current value in HL
			uint16_t value = cpu_regs.BC; // get current value from BC

			if ((old_HL & 0x0FFF)+(value & 0x0FFF) > 0x0FFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			if (((uint32_t)old_HL + (uint32_t)value) > 0xFFFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

			cpu_regs.HL += cpu_regs.BC;
		}
		break;

		case 0x19: //add hl, DE
		{
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);

			uint16_t old_HL = cpu_regs.HL; // get current value in HL
			uint16_t value = cpu_regs.DE; // get current value from BC

			if ((old_HL & 0x0FFF)+(value & 0x0FFF) > 0x0FFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			if (((uint32_t)old_HL + (uint32_t)value) > 0xFFFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

			cpu_regs.HL += cpu_regs.DE;
		}
		break;

		case 0x29: //add hl, HL
		{
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);

			uint16_t old_HL = cpu_regs.HL; // get current value in HL
			uint16_t value = cpu_regs.HL; // get current value from BC

			if ((old_HL & 0x0FFF)+(value & 0x0FFF) > 0x0FFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			if (((uint32_t)old_HL + (uint32_t)value) > 0xFFFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

			cpu_regs.HL += cpu_regs.HL;
		}
		break;


		case 0x39: //add hl, SP
		{
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);

			uint16_t old_HL = cpu_regs.HL; // get current value in HL
			uint16_t value = cpu_regs.SP; // get current value from BC

			if ((old_HL & 0x0FFF)+(value & 0x0FFF) > 0x0FFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			if (((uint32_t)old_HL + (uint32_t)value) > 0xFFFF)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

			cpu_regs.HL += cpu_regs.SP;
		}
		break;


		case 0x04: // INC B
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.B; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.B++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.B after incrementing
			if (cpu_regs.B == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x0C: // INC C
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.C; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.C++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.C after incrementing
			if (cpu_regs.C == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x14: // INC D
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.D; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.D++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.D after incrementing
			if (cpu_regs.D == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x1C: // INC E
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.E; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.E++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.E after incrementing
			if (cpu_regs.E == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x24: // INC H
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.H; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.H++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.H after incrementing
			if (cpu_regs.H == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x2C: // INC L
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.L; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.L++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.LH after incrementing
			if (cpu_regs.L == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x34: // INC (HL) - Increment byte at memory address (HL)
		{
			// N Flag (Subtract): Always reset (0)
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);

			uint16_t hl_address = cpu_regs.HL; // Get the memory address from HL
			uint8_t old_value_at_HL = mmu_read_byte(hl_address); // Read the byte from memory

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			if ((old_value_at_HL & 0x0F) == 0x0F) {
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			} else {
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment on the value from memory
			uint8_t new_value_at_HL = old_value_at_HL + 1; // Or old_value_at_HL++;

			// Z Flag (Zero): Set if the result is 0x00
			if (new_value_at_HL == 0x00) {
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			} else {
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED

			// Write the incremented value back to memory
			mmu_write_byte(hl_address, new_value_at_HL);
		}
		break;

		case 0x3C: // INC A
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.A; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x0F)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.A++;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.A after incrementing
			if (cpu_regs.A == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;


		case 0x05: // DEC B
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.B; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.B--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.B after incrementing
			if (cpu_regs.B == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x0D: // DEC C
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.C; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.C--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.C after incrementing
			if (cpu_regs.C == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x15: // DEC D
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.D; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.D--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.D after incrementing
			if (cpu_regs.D == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x1D: // DEC E
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.E; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.E--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.E after incrementing
			if (cpu_regs.E == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x25: // DEC H
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.H; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.H--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.H after incrementing
			if (cpu_regs.H == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x2D: // DEC L
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.L; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.L--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.LH after incrementing
			if (cpu_regs.L == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;

		case 0x35: // DEC (HL) - Increment byte at memory address (HL)
		{
			// N Flag (Subtract): Always reset (0)
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);

			uint16_t hl_address = cpu_regs.HL; // Get the memory address from HL
			uint8_t old_value_at_HL = mmu_read_byte(hl_address); // Read the byte from memory

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			if ((old_value_at_HL & 0x0F) == 0x00) {
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			} else {
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment on the value from memory
			uint8_t new_value_at_HL = old_value_at_HL - 1; // Or old_value_at_HL++;

			// Z Flag (Zero): Set if the result is 0x00
			if (new_value_at_HL == 0x00) {
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			} else {
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED

			// Write the incremented value back to memory
			mmu_write_byte(hl_address, new_value_at_HL);
		}
		break;

		case 0x3D: // DEC A
		{
			// N Flag (Subtract): Always reset (0) for INC r8
			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT); // Use FLAG_N_POS as per our macro discussion

			uint8_t old_value = cpu_regs.A; // Store original value for Half Carry check

			// H Flag (Half Carry): Set if carry from bit 3 to bit 4
			// This occurs if the lower nibble (bits 0-3) was 0xF before incrementing.
			if ((old_value & 0x0F) == 0x00)
			{ // Check if the lower nibble is 0xF
				SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
			}

			// Perform the actual increment
			cpu_regs.A--;

			// Z Flag (Zero): Set if the result is 0x00
			// Check the NEW value of cpu_regs.A after incrementing
			if (cpu_regs.A == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}

			// C Flag (Carry): NOT AFFECTED by INC r8
		}
		break;


		case 0x06: //ld r8 imm8
		{
			cpu_regs.B = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x0E: //ld r8 imm8
		{
			cpu_regs.C = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x16: //ld r8 imm8
		{
			cpu_regs.D = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x1E: //ld r8 imm8
		{
			cpu_regs.E = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x26: //ld r8 imm8
		{
			cpu_regs.H = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x2E: //ld r8 imm8
		{
			cpu_regs.L = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x36: //ld r8 imm8
		{
			mmu_write_byte(cpu_regs.HL, mmu_read_byte(cpu_regs.PC));
			cpu_regs.PC++;
		}
		break;

		case 0x3E: //ld r8 imm8
		{
			cpu_regs.A = mmu_read_byte(cpu_regs.PC);
			cpu_regs.PC++;
		}
		break;

		case 0x07: //Rotate Left Circular Accumulator
		{

			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);

			uint8_t value = cpu_regs.A; // Store original value for Half Carry check

			// H Flag (Half Carry): Not affected

			if (CHK_BIT(value, 7))
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

			// Perform the actual increment

			cpu_regs.A = (cpu_regs.A << 1 | value >> 7);

		}
		break;

		case 0x0F: //rrca
		{

			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);

			uint8_t value = cpu_regs.A; // Store original value for Half Carry check

			// H Flag (Half Carry): Not affected

			if (CHK_BIT(value, 0))
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

			// Perform the actual increment

			cpu_regs.A = (cpu_regs.A >> 1 | value << 7);

		}
		break;

		case 0x17: //rla
		{
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);

			uint8_t value = cpu_regs.A; // Store original value for Half Carry check

			cpu_regs.A = cpu_regs.A << 1;
			if (CHK_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT))
			{
				SET_BIT(cpu_regs.A, 0);
			}
			else
			{
				CLR_BIT(cpu_regs.A, 0);
			}

			// H Flag (Half Carry): Not affected

			if (CHK_BIT(value, 7))
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

		}
		break;

		case 0x1F: //rra
		{
			CLR_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);

			uint8_t value = cpu_regs.A; // Store original value for Half Carry check

			cpu_regs.A = cpu_regs.A >> 1;
			if (CHK_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT))
			{
				SET_BIT(cpu_regs.A, 7);
			}
			else
			{
				CLR_BIT(cpu_regs.A, 7);
			}

			// H Flag (Half Carry): Not affected

			if (CHK_BIT(value, 0))
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}
			else
			{
				CLR_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
			}

		}
		break;

		case 0x27: // DAA - Decimal Adjust Accumulator
		{
			// Capture the flags from the instruction executed immediately before DAA.
			uint8_t flags_from_previous_instruction = cpu_regs.F;

			// The Half-Carry (H) flag is always cleared by DAA.
			CLR_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);

			// Branch based on whether the previous operation was subtraction (N=1) or addition (N=0).
			if (CHK_BIT(flags_from_previous_instruction, CPU_FLAG_SUB_N_BIT))
			{
				/**
				 * Subtraction Logic (N flag was SET):
				 * Adjusts Accumulator (A) and flags after a binary subtraction to make it BCD.
				 * Adjustments depend ONLY on the H and C flags from the previous instruction.
				 */

				// Check if Half-Carry (H) flag was set from previous instruction.
				if( CHK_BIT(flags_from_previous_instruction, CPU_FLAG_HALF_H_BIT))
				{
					// Subtract 0x06 from Accumulator if H was set.
					cpu_regs.A -= 0x06;
				}

				// Check if Carry (C) flag was set from previous instruction.
				if(CHK_BIT(flags_from_previous_instruction, CPU_FLAG_CARRY_C_BIT))
				{
					// Subtract 0x60 from Accumulator if C was set.
					cpu_regs.A -= 0x60;
					// This line needs review for subtraction DAA.
				}
			}
			else // N_FLAG is CLEAR, meaning an ADDITION was performed.
			{
				/**
				 * Addition Logic (N flag was CLEAR):
				 * Adjusts Accumulator (A) and flags after a binary addition to make it BCD.
				 */

				// Adjust lower nibble (bits 0-3) if H flag set OR lower nibble > 0x09.
				if( (CHK_BIT(flags_from_previous_instruction, CPU_FLAG_HALF_H_BIT) ) || ((cpu_regs.A & 0x0F) > 0x09))
				{
					// Add 0x06 to the Accumulator.
					cpu_regs.A += 0x06;
				}

				// Adjust upper nibble (bits 4-7) if C flag set OR Accumulator > 0x99.
				// (Uses cpu_regs.A's value after potential lower nibble adjustment).
				if((CHK_BIT(flags_from_previous_instruction, CPU_FLAG_CARRY_C_BIT) ) || (cpu_regs.A > 0x99))
				{
					// Add 0x60 to the Accumulator.
					cpu_regs.A += 0x60;
					// Set the Carry (C) flag.
					SET_BIT(cpu_regs.F, CPU_FLAG_CARRY_C_BIT);
				}
			}

			// --- Final Flag Updates ---

			// Clear Zero (Z) flag.
			CLR_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);

			// Set Zero (Z) flag if Accumulator is 0x00.
			if(cpu_regs.A == 0x00)
			{
				SET_BIT(cpu_regs.F, CPU_FLAG_ZERO_Z_BIT);
			}
			// N (Subtract) flag is UNCHANGED by DAA.
			// H (Half Carry) flag is always cleared by DAA.
			// C (Carry) flag is handled within addition/subtraction logic.
		}
		break;

		case 0x2F: //cpl
		{
			uint8_t to_be_notted = cpu_regs.A;
			cpu_regs.A = ~to_be_notted;

			SET_BIT(cpu_regs.F, CPU_FLAG_SUB_N_BIT);
			SET_BIT(cpu_regs.F, CPU_FLAG_HALF_H_BIT);
		}
		break;

		case 0x37: //scf
		{}break;
		case 0x3F: //ccf
		{}break;



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

