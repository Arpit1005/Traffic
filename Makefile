# Makefile for TrafficGuru

# --- Variables ---

# Compiler
CC = gcc

# Directories
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin
INC_DIR = include

# Target executable
TARGET = $(BIN_DIR)/trafficguru

# Find all .c source files in the src directory
SOURCES = $(wildcard $(SRC_DIR)/*.c)

# Create a list of object files in the obj directory
# e.g., src/main.c -> obj/main.o
OBJECTS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))

# Compiler Flags
# -Wall -Wextra: Enable all and extra warnings (helps find bugs)
# -g: Include debug symbols (for GDB)
# -I$(INC_DIR): Tell GCC to look for headers in the 'include' directory
# -std=c99: Use the C99 standard
# -D_XOPEN_SOURCE=600: Define _XOPEN_SOURCE to match your source files (fixes warnings)
# -pthread: Include thread-safe compilation
CFLAGS = -Wall -Wextra -g -I$(INC_DIR) -std=c99 -D_XOPEN_SOURCE=600 -pthread

# Linker Flags
# -lncurses: Link the ncurses library (for the terminal UI)
# -pthread: Link the POSIX threads library (for multithreading)
# -lm: Link the math library
LDFLAGS = -lncurses -pthread -lm

# --- Rules ---

# Default target: 'make all' or just 'make'
all: $(TARGET)

# Rule to build the final executable
# Depends on all object files and the bin directory
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	@echo "Linking $(TARGET)..."
	$(CC) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	@echo "Build complete! Run with 'make run' or './$(TARGET)'"

# Rule to compile a .c file into a .o file
# Depends on the source .c file and the obj directory
# $< is the source file (e.g., src/main.c)
# $@ is the target file (e.g., obj/main.o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	@echo "Compiling $<..."
	$(CC) $(CFLAGS) -c $< -o $@

# Rules to create the bin and obj directories if they don't exist
# These are 'order-only prerequisites' (using |)
$(BIN_DIR):
	@echo "Creating directory $(BIN_DIR)..."
	@mkdir -p $(BIN_DIR)

$(OBJ_DIR):
	@echo "Creating directory $(OBJ_DIR)..."
	@mkdir -p $(OBJ_DIR)

# --- Utility Rules ---

# 'make run' - Build and run the executable
run: $(TARGET)
	@echo "Running $(TARGET)..."
	./$(TARGET)

# 'make clean' - Remove build artifacts
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(OBJ_DIR) $(BIN_DIR)
	@echo "Clean complete."

# 'make help' - From your README
help:
	@echo "Available make targets:"
	@echo "  make (or make all) - Build the project"
	@echo "  make run             - Build and run the project"
	@echo "  make clean           - Remove all build artifacts"
	@echo "  make help            - Show this help message"

# Phony targets are not files
.PHONY: all run clean help
