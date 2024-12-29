SRC_DIR = src
BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj

CC = cc
CFLAGS = -Wall -Wextra -O2 -std=c99 -g

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

TARGET = formula-analyzer
TARGET_PATH = $(BUILD_DIR)/$(TARGET)

RED = \033[0;31m
GREEN = \033[0;32m
YELLOW = \033[0;33m
NC = \033[0m
f=""

default: run

compile: $(TARGET_PATH)

$(TARGET_PATH): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo -e "${GREEN}Linking $(TARGET)${NC}"
	@$(CC) $(CFLAGS) -o $(TARGET_PATH) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo -e "${YELLOW}Compiling $<${NC}"
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@echo -e "$(YELLOW)Done.$(NC)"

run: compile
	@cd $(BUILD_DIR) && ./$(TARGET) $(f)

valgrind: compile
	@cd $(BUILD_DIR) && valgrind  --leak-check=full --show-leak-kinds=all $(TARGET) $(f)

.PHONY: compile
