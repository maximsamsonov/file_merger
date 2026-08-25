CC = gcc
CFLAGS = -O2 -pthread -Wall -Wextra -Werror
TARGET = solution
SRC_DIR = src
BUILD_DIR = build

SRCS = $(SRC_DIR)/utils.c $(SRC_DIR)/hash.c $(SRC_DIR)/load.c \
       $(SRC_DIR)/sort.c $(SRC_DIR)/inc.c $(SRC_DIR)/search.c \
       $(SRC_DIR)/save.c $(SRC_DIR)/test.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

test: $(TARGET)
	./$(TARGET)

.PHONY: all clean test

