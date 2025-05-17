CC := gcc
CFLAGS := -Wall --pedantic -fsanitize=address -Wextra -std=c11 -O2 -I. -Isrc
debug: CFLAGS := -Wall --pedantic -fsanitize=address -Wextra -std=c11 -g -O0 -I. -Isrc


SRC_DIR := src
OBJ_DIR = build

SRCS = $(shell find $(SRC_DIR) -name "*.c" ! -path "$(SRC_DIR)/test/*")
HDRS = $(shell find $(SRC_DIR) -name "*.h" ! -path "$(SRC_DIR)/test/*")
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o)

TARGET := bin/app

CLANG_TIDY = clang-tidy
CLANG_TIDY_OPTS = --quiet

.PHONY: all clean lint

all: $(TARGET)

debug: clean all

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

lint:
	@echo "Running clang-tidy on source files..."
	@$(foreach file, $(SRCS) $(HDRS), \
		$(CLANG_TIDY) $(file) $(CLANG_TIDY_OPTS) -- -I. -Isrc || exit 1;)
