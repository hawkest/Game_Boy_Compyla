🎮 Game Boy Compyla (C)
This project is an ongoing endeavor to build a functional Nintendo Game Boy emulator from scratch using the C programming language. It serves as a deep dive into low-level system programming, exploring classic computer architecture, and understanding how retro consoles operate at a fundamental level.

✨ Project Goals
Explore C Programming: Gain a deeper understanding of C, including memory management, pointers, and efficient code organization.

Understand Old Architectures: Demystify the inner workings of 8-bit systems, specifically the Game Boy's LR35902 CPU and its associated hardware.

Accurate Emulation: Strive for cycle-accurate or near-cycle-accurate emulation of the Game Boy's components.

🚀 Current Status & Implemented Features
The emulator is in active development. So far, the focus has been on establishing foundational components:

Memory Management Unit (MMU):

Full memory map definition.

Accurate mmu_read_byte and mmu_write_byte implementations, including Echo RAM mirroring and handling of ignored memory regions.

ROM loading (mmu_load_rom) for 32 KiB cartridges.

CPU (LR35902):

CPU state and register definitions.

Correct CPU initialization to post-boot ROM state.

Core fetch-decode-execute cycle implemented (cpu_run, cpu_step).

Initial instruction implementations (e.g., NOP, LD r16, imm16, LD (r16), A).

🚧 Upcoming Features
CPU Instruction Set Expansion: Implement the full Game Boy CPU instruction set.

Picture Processing Unit (PPU): Graphics rendering, sprites, backgrounds, and LCD control.

Interrupt System: Handling CPU interrupts for various events.

Timer System: Accurate emulation of the DIV, TIMA, TMA, and TAC registers.

Input Handling: Reading joypad input.

Sound Emulation: Replicating the Game Boy's audio output.

Memory Bank Controllers (MBCs): Support for larger ROMs.

🛠️ Building the Project (Eclipse CDT)
This project is developed using Eclipse CDT.

Clone the Repository:

git clone https://github.com/hawkest/Game_Boy_Compyla.git
cd Game_Boy_Compyla

Import into Eclipse:

Open Eclipse.

Go to File > Import... > General > Existing Projects into Workspace.

Select the root directory of the cloned repository.

Click Finish.

Configure C Standard (if needed):

Right-click on the project in Project Explorer > Properties > C/C++ Build > Settings > Tool Settings tab > GCC C Compiler > Dialect.

Set "C Standard" to ISO C99 (-std=c99).

Click Apply and Close.

Build:

Right-click on the project > Build Project.

🏃 Usage
(Instructions on how to run your emulator will go here once it's capable of loading and running a ROM to a visible state. For now, it's primarily for development and debugging.)

🤝 Contributing
Contributions are welcome! If you find bugs, have suggestions, or want to add features, please open an issue or submit a pull request.

📄 License
This project is licensed under the MIT License.
