# ===== Project =====
LIB_NAME        := libda.a
TEST_NAME       := test_da

# ===== Directories =====
INC_DIR         := include
SRC_DIR         := src
TEST_DIR        := tests

# ===== Platform =====
UNAME_S         := $(shell uname -s)

# ===== Toolchain =====
CC              := clang
AR              := ar
ARFLAGS         := rcs

# ===== Build type =====
BUILD           ?= debug   # debug or release
BUILD           := $(strip $(BUILD))

# ===== Common flags =====
CSTD            := -std=c11
WARNINGS        := -Wall -Wextra -Werror -pedantic
CPPFLAGS        := -I$(INC_DIR)

# ===== Build-specific flags & output dirs =====
ifeq ($(BUILD),debug)
CFLAGS          := $(CSTD) $(WARNINGS) -g -O0 -fsanitize=address,undefined
LDFLAGS         := -fsanitize=address,undefined
OUT_DIR         := build/debug
else ifeq ($(BUILD),release)
CFLAGS          := $(CSTD) $(WARNINGS) -O2 -DNDEBUG
LDFLAGS         :=
OUT_DIR         := build/release
else
$(error Unknown BUILD type: '$(BUILD)' (use BUILD=debug or BUILD=release))
endif

OBJ_DIR         := $(OUT_DIR)/obj
BIN_DIR         := $(OUT_DIR)/bin
LIB_DIR         := $(OUT_DIR)/lib

LIB_TARGET      := $(LIB_DIR)/$(LIB_NAME)
TEST_TARGET     := $(BIN_DIR)/$(TEST_NAME)

# ===== Files =====
LIB_SRCS        := $(SRC_DIR)/da.c
TEST_SRCS       := $(TEST_DIR)/test_da.c

LIB_OBJS        := $(LIB_SRCS:%.c=$(OBJ_DIR)/%.o)
TEST_OBJS       := $(TEST_SRCS:%.c=$(OBJ_DIR)/%.o)

# ===== Targets =====
.PHONY: all lib test run leaks valgrind clean re

all: lib

lib: $(LIB_TARGET)

$(LIB_TARGET): $(LIB_OBJS)
	@mkdir -p $(LIB_DIR)
	$(AR) $(ARFLAGS) $@ $^

test: $(TEST_TARGET)

$(TEST_TARGET): $(TEST_OBJS) $(LIB_TARGET)
	@mkdir -p $(BIN_DIR)
	$(CC) $(TEST_OBJS) $(LIB_TARGET) $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(CPPFLAGS) -c $< -o $@

run: test
	./$(TEST_TARGET)

leaks: test
ifeq ($(UNAME_S),Darwin)
	leaks -quiet -atExit -- ./$(TEST_TARGET)
else
	@echo "leaks is macOS-only. Use 'make valgrind' on Linux."
endif

valgrind: test
ifeq ($(UNAME_S),Linux)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./$(TEST_TARGET)
else
	@echo "valgrind is Linux-only. Use 'make leaks' on macOS."
endif

clean:
	rm -rf build

re: clean all

