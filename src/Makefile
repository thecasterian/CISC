CC = gcc
CFLAGS = -Wall -Wextra -O0 -g -rdynamic

TARGET = cisc
OBJS = lexer.o utils.o

all: $(TARGET)

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

$(TARGET): cisc.c $(OBJS)
	$(CC) $^ -o $@ $(CFLAGS)
	mv $@ ../

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)
