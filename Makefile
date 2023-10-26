OUTPUT = exec

SOURCES = main.cpp position.cpp bitboard.cpp tools.cpp typesdef.h

CC = g++

CFLAGS = -Wall -std=c++11

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	$(CC) $(CFLAGS) -o $(OUTPUT) $(SOURCES)

clean:
	rm -f $(OUTPUT)
