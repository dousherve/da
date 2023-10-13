CC = gcc
CFLAGS = -Wall -Wextra -Werror

SOURCES = da.c dap.c sb.c
LIBRARY = libda.a

TEST_SOURCES = test/da_test.c
TEST = test/da_test

OBJ_DIR = obj
OBJECTS = $(addprefix $(OBJ_DIR)/, $(SOURCES:.c=.o))
TEST_OBJECTS = $(TEST_SOURCES:.c=.o)

all: $(LIBRARY)

$(LIBRARY): $(OBJECTS)
	ar rcs $(LIBRARY) $(OBJECTS)
	ranlib $(LIBRARY)

$(OBJ_DIR)/%.o: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

test: $(TEST) $(LIBRARY)

$(TEST): $(TEST_OBJECTS) $(LIBRARY)
	$(CC) $(CFLAGS) -o $(TEST) $(TEST_OBJECTS) -L. -lda

test/%.o: test/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(LIBRARY) $(OBJ_DIR) $(TEST) $(TEST_OBJECTS)

re: clean all

.PHONY: all test clean re
