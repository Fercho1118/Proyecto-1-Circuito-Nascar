# Compilacion del screensaver. Los fuentes viven en src/ y los objetos
# intermedios en build/, de modo que el arbol de trabajo se mantiene limpio.

TARGET  := nascar
SRC_DIR := src
OBJ_DIR := build

CC      := clang
CFLAGS  := -std=c11 -Wall -Wextra -O2 $(shell sdl2-config --cflags)
LDFLAGS := $(shell sdl2-config --libs) -lm

# En macOS clang no trae OpenMP integrado; se toma de libomp de Homebrew si
# esta instalada. Sin ella el proyecto compila igual, en version secuencial.
OMP_PREFIX := $(shell brew --prefix libomp 2>/dev/null)
ifneq ($(OMP_PREFIX),)
CFLAGS  += -Xpreprocessor -fopenmp -I$(OMP_PREFIX)/include
LDFLAGS += -L$(OMP_PREFIX)/lib -lomp
endif

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

# -MMD -MP generan los archivos .d con las dependencias de cada objeto, para
# que tocar un encabezado recompile solo lo que de verdad lo incluye.
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(OBJ_DIR) $(TARGET)

-include $(DEPS)
