# Project name
NAME = primex_monitor

# Define compiler and flags
G++32 = i686-w64-mingw32-g++
G++64 = x86_64-w64-mingw32-g++
GCC32 = i686-w64-mingw32-gcc
GCC64 = x86_64-w64-mingw32-gcc

G++FLAGS = -std=c++11
GCCFLAGS = -std=c11
LDFLAGS = -lpthread -static -static-libgcc -static-libstdc++

# Architecture flags
ARCH_X86 = -m32
ARCH_X64 = -m64

# Directories
SRC_DIR = src
BUILD_DIR = build
INCLUDE_DIR = include

# Files
SQLITE_HEADER = $(SRC_DIR)/sqlite3.h
SQLITE_SRC = $(SRC_DIR)/sqlite3.c
MAIN_SRC = $(SRC_DIR)/main.cpp
OBJ_DIR = $(BUILD_DIR)/obj

# Output executable names
EXE_X86 = $(BUILD_DIR)/$(NAME)_x86
EXE_X64 = $(BUILD_DIR)/$(NAME)_x64

# Create object directory
$(shell mkdir -p $(OBJ_DIR))

# Default target (build both x86 and x64)
all: $(EXE_X86) $(EXE_X64)

# x86 (32-bit) target
$(EXE_X86): $(OBJ_DIR)/main_x86.o $(OBJ_DIR)/sqlite3_x86.o
	$(G++32) $(ARCH_X86) -o $@ $^ $(LDFLAGS)

# x64 (64-bit) target
$(EXE_X64): $(OBJ_DIR)/main_x64.o $(OBJ_DIR)/sqlite3_x64.o
	$(G++64) $(ARCH_X64) -o $@ $^ $(LDFLAGS)

# Compile the main source for x86
$(OBJ_DIR)/main_x86.o: $(MAIN_SRC) $(SQLITE_HEADER)
	$(G++32) $(ARCH_X86) $(G++FLAGS) -I$(SRC_DIR) -c $(MAIN_SRC) -o $@

# Compile the main source for x64
$(OBJ_DIR)/main_x64.o: $(MAIN_SRC) $(SQLITE_HEADER)
	$(G++64) $(ARCH_X64) $(G++FLAGS) -I$(SRC_DIR) -c $(MAIN_SRC) -o $@

# Compile sqlite3.c for x86
$(OBJ_DIR)/sqlite3_x86.o: $(SQLITE_SRC) $(SQLITE_HEADER)
	$(GCC32) $(ARCH_X86) $(GCCFLAGS) -I$(SRC_DIR) -c $(SQLITE_SRC) -o $@

# Compile sqlite3.c for x64
$(OBJ_DIR)/sqlite3_x64.o: $(SQLITE_SRC) $(SQLITE_HEADER)
	$(GCC64) $(ARCH_X64) $(GCCFLAGS) -I$(SRC_DIR) -c $(SQLITE_SRC) -o $@

# Clean build files
clean:
	rm -rf $(OBJ_DIR) $(BUILD_DIR)

# Phony targets (not actual files)
.PHONY: all clean
