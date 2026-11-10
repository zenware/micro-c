CC       = clang
CFLAGS   = -std=c11 -Wall -Wextra -Wpedantic -g
LDFLAGS  =

TARGET   = microcc
SOURCE   = microcc.c
TEST     = test.sh

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(SOURCE)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

test: $(TARGET)
	./$(TEST)

clean:
	rm -f $(TARGET) generated.s generated *.o *~
