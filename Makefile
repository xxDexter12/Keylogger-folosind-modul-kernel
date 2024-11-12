# Numele executabilului
TARGET := init

# Sursele și fișierele obiect
SRC_FILES := $(wildcard *.c)
OBJ_FILES := $(SRC_FILES:.c=.o)

# Compiler și flaguri
CC := gcc
CFLAGS := -Wall -Wextra -O2
#ccflags-y := $(ccflags-y) -xc -E -v

# Reguli
.PHONY: all clean

# Regula principală de compilare
all: $(TARGET)

# Crearea executabilului
$(TARGET): $(OBJ_FILES)
	$(CC) $(CFLAGS) $(ccflags-y) -o $@ $^

# Compilarea fișierelor sursă individuale
%.o: %.c
	$(CC) $(CFLAGS) $(ccflags-y) -c $< -o $@

# Curățare
clean:
	rm -f *.o $(TARGET)