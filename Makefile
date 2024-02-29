
CC = gcc
CFLAGS = -g -O3 -std=c++17 -march=native -DNDEBUG
TARGET = BinpacktoMarlin

release: $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)