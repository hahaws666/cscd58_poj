# 编译器
# gcc
CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
LDFLAGS = -lpthread
SRCS = $(wildcard src/*.c)
TARGET = monitor
all: $(TARGET)
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)