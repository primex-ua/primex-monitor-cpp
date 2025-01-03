# Variables
CC = g++
SRC_DIR = src
LIB_DIR = lib
BIN_DIR = bin
OBJ_DIR = $(BIN_DIR)/obj
TARGET = $(BIN_DIR)/primex_monitor.exe
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))
LIBS = $(wildcard $(LIB_DIR)/*.a)
CFLAGS = -lpthread -static -static-libgcc -static-libstdc++

# Default target
all: $(TARGET)

# Build target
$(TARGET): $(OBJS) $(LIBS)
	@mkdir -p $(BIN_DIR)
	$(CC) -o $@ $(OBJS) $(LIBS) $(CFLAGS)

# Build object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) -c $< -o $@

# Clean target
clean:
	rm -rf $(BIN_DIR)

run:
	./$(TARGET)

.PHONY: all clean