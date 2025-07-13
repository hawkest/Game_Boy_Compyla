# Minimum required CMake version
cmake_minimum_required(VERSION 3.10)

# Define the project name and specify C as the language
project(Game_Boy_Compyla LANGUAGES C)

# Set the C standard (e.g., C11, C99). Good practice for C projects.
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define the executable target.
# The first argument is the name of your executable (e.g., GameBoyEmulator.exe).
# The subsequent arguments are your source files.
add_executable(GameBoyEmulator main.c)