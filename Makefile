TARGET := nascar

CC      := clang
CFLAGS  := -std=c11 -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lm

# Apple clang no trae OpenMP: necesita -Xpreprocessor y la libomp de Homebrew.
# En otras plataformas basta con -fopenmp.
OMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
ifneq ($(OMP_PREFIX),)
  CFLAGS  += -Xpreprocessor -fopenmp -I$(OMP_PREFIX)/include
  LDFLAGS += -L$(OMP_PREFIX)/lib -lomp
else
  CFLAGS  += -fopenmp
  LDFLAGS += -fopenmp
endif

.PHONY: all run clean

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) main.c -o $@ $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
