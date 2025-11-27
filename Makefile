# Makefile for Network Monitor Project

# 编译器
CC = gcc

# 编译选项
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -lpthread

# 目录
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# 源文件
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/monitor.c \
          $(SRC_DIR)/stats.c \
          $(SRC_DIR)/icmp.c \
          $(SRC_DIR)/tcp_scan.c

# 目标文件
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# 可执行文件
TARGET = $(BIN_DIR)/monitor

# 包含路径
INCLUDES = -I$(SRC_DIR) -I$(INCLUDE_DIR)

# 默认目标
all: $(TARGET)

# 创建目录
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# 链接可执行文件
$(TARGET): $(OBJECTS) | $(BIN_DIR)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# 编译源文件为目标文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# 清理
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "Clean complete"

# 重新编译
rebuild: clean all

# 安装（设置 setuid 以便运行 raw socket，需要 root 权限）
install: $(TARGET)
	@echo "Installing $(TARGET)..."
	@echo "Note: This program requires root privileges to use raw sockets"
	@echo "You may need to run: sudo chmod +s $(TARGET)"

# 运行
run: $(TARGET)
	@echo "Note: This program requires root privileges to use raw sockets"
	@echo "Run with: sudo $(TARGET)"

# 帮助信息
help:
	@echo "Available targets:"
	@echo "  all      - Build the project (default)"
	@echo "  clean    - Remove build artifacts"
	@echo "  rebuild  - Clean and rebuild"
	@echo "  install  - Install the binary (may need setuid)"
	@echo "  run      - Show how to run the program"
	@echo "  help     - Show this help message"

.PHONY: all clean rebuild install run help

