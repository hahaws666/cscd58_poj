# 编译器
# gcc
CC = gcc

# 编译选项
# options
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -lpthread

# 目录
# menu
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
BIN_DIR = bin

# 源文件
SOURCES = $(SRC_DIR)/main.c \
          $(SRC_DIR)/monitor.c \
          $(SRC_DIR)/stats.c \
          $(SRC_DIR)/icmp.c \
          $(SRC_DIR)/tcp_scan.c \
          $(SRC_DIR)/data_analysis.c

# 目标文件
# target files
OBJECTS = $(SOURCES:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# 可执行文件
# bin files
TARGET = $(BIN_DIR)/monitor

# 包含路径
# all paths all the ways
INCLUDES = -I$(SRC_DIR) -I$(INCLUDE_DIR)

# 默认目标
# default target (TARGET)
all: $(TARGET)

# 创建目录
# build the directory
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


