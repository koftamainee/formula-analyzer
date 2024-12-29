SRC_DIR = src
INCLUDE_DIR = libc
BUILD_DIR = target
OBJ_DIR = $(BUILD_DIR)/obj

CC = cc
CFLAGS = -Wall -Wextra -O2 -std=c99 -g -lm

SRCS += $(wildcard $(SRC_DIR)/*.c)
SRCS += $(wildcard $(INCLUDE_DIR)/src/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(filter $(SRC_DIR)/%,$(SRCS))) \
        $(patsubst $(INCLUDE_DIR)/src/%,$(OBJ_DIR)/%.o,$(filter $(INCLUDE_DIR)/src/%,$(SRCS)))

TOTAL := $(shell echo $(SRCS) | wc -w)

TARGET = formula-analyzer
TARGET_PATH = $(BUILD_DIR)/$(TARGET)

COLOR_RED = \033[0;31m
COLOR_GREEN = \033[0;32m
COLOR_YELLOW = \033[0;33m
COLOR_RESET = \033[0m
f=""

default: run

compile: $(TARGET_PATH)

$(TARGET_PATH): $(OBJS)
	@mkdir -p $(BUILD_DIR)
	@echo -e "${COLOR_GREEN}Linking $(TARGET)${COLOR_RESET}"
	@$(CC) $(CFLAGS) -o $(TARGET_PATH) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@CURRENT=$$(expr $(shell echo $(OBJS) | tr ' ' '\n' | grep -n "$@" | cut -d: -f1) + 0); \
	echo -e "[$$CURRENT/$(TOTAL)] $(COLOR_GREEN)Building C object $@$(COLOR_RESET)"; \
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(INCLUDE_DIR)/src/%
	@mkdir -p $(OBJ_DIR)
	@CURRENT=$$(expr $(shell echo $(OBJS) | tr ' ' '\n' | grep -n "$@" | cut -d: -f1) + 0); \
	echo -e "[$$CURRENT/$(TOTAL)] $(COLOR_GREEN)Building C object $@$(COLOR_RESET)"; \
	$(CC) $(CFLAGS) -I$(INCLUDE_DIR) -c $< -o $@

clean:
	@rm -rf $(BUILD_DIR)
	@echo -e "$(COLOR_YELLOW)Done.$(COLOR_RESET)"

run: compile
	@cd $(BUILD_DIR) && ./$(TARGET) $(f)

valgrind: compile
	@cd $(BUILD_DIR) && valgrind  --leak-check=full --show-leak-kinds=all $(TARGET) $(f)

.PHONY: compile
