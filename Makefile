CC       = clang
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -g
LDFLAGS  =

TARGET   = one_integer
SOURCE   = one_integer.c
TEST     = test.sh

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(TARGET)
	./$(TEST)

clean:
	rm -f $(TARGET) generated.s generated *.o *~
