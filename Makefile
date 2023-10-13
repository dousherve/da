CC = gcc
CFLAGS = -Wall -Wextra -Werror -I da
LDFLAGS = -L. -lda

LIBRARY = libda.a
TEST = test/da_test

SRC_DIR = src
TEST_DIR = test
OBJ_DIR = obj

# List of source files for the library
LIB_SRC = $(wildcard $(SRC_DIR)/*.c)
LIB_OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(LIB_SRC))

# List of source files for the test program
TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_OBJ = $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.o,$(TEST_SRC))

all: $(LIBRARY) $(TEST)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(LIBRARY): $(LIB_OBJ)
	ar rcs $@ $(LIB_OBJ)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): $(TEST_OBJ) | $(LIBRARY)
	$(CC) $(CFLAGS) -o $@ $(TEST_OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(TEST_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(LIBRARY) $(TEST) $(OBJ_DIR)

test: $(TEST)

re: clean all

.PHONY: all clean test re
