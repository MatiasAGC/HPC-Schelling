CC := gcc
MPICC := mpicc
MPIEXEC := mpirun
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Iinclude
RELEASE_FLAGS := -O3 -DNDEBUG
DEBUG_FLAGS := -O0 -g3
OPENMP_FLAGS := -fopenmp
BUILD_DIR := build-make
COMMON_SOURCES := src/common/aleatorio.c src/common/argumentos.c src/common/configuracion.c \
	src/common/economia.c src/common/generador.c src/common/modelo.c src/common/registro.c \
	src/common/vecindario.c src/sequential/simulacion.c
COMMON_OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/make/%.o,$(COMMON_SOURCES))
VERSION := 0.1.0
CPPFLAGS := -DSCHELLING_VERSION=\"$(VERSION)\"

.PHONY: all release debug test format format-check analyze clean

all: release

release: CFLAGS += $(RELEASE_FLAGS)
release: $(BUILD_DIR)/schelling_seq $(BUILD_DIR)/schelling_hybrid

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(BUILD_DIR)/schelling_seq $(BUILD_DIR)/schelling_hybrid

$(BUILD_DIR)/schelling_seq: app/schelling_seq.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -lm -o $@

$(BUILD_DIR)/schelling_hybrid: app/schelling_hybrid.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(MPICC) $(CPPFLAGS) $(CFLAGS) $(OPENMP_FLAGS) $^ -lm -o $@

$(BUILD_DIR)/make/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/prueba_configuracion: tests/unit/prueba_configuracion.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) \
		-DRUTA_CONFIGURACION_PRUEBA=\"tests/data/minima.conf\" $^ -lm -o $@

$(BUILD_DIR)/prueba_modelo: tests/unit/prueba_modelo.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $^ -lm -o $@

$(BUILD_DIR)/prueba_aleatorio: tests/unit/prueba_aleatorio.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $^ -lm -o $@

$(BUILD_DIR)/prueba_generador: tests/unit/prueba_generador.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $^ -lm -o $@

$(BUILD_DIR)/prueba_economia: tests/unit/prueba_economia.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $^ -lm -o $@

$(BUILD_DIR)/prueba_simulacion: tests/unit/prueba_simulacion.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $^ -lm -o $@

$(BUILD_DIR)/prueba_vecindario: tests/unit/prueba_vecindario.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) $^ -lm -o $@

test: debug $(BUILD_DIR)/prueba_configuracion $(BUILD_DIR)/prueba_aleatorio \
	$(BUILD_DIR)/prueba_economia $(BUILD_DIR)/prueba_generador $(BUILD_DIR)/prueba_modelo \
	$(BUILD_DIR)/prueba_simulacion $(BUILD_DIR)/prueba_vecindario
	$(BUILD_DIR)/prueba_configuracion
	$(BUILD_DIR)/prueba_aleatorio
	$(BUILD_DIR)/prueba_economia
	$(BUILD_DIR)/prueba_generador
	$(BUILD_DIR)/prueba_modelo
	$(BUILD_DIR)/prueba_simulacion
	$(BUILD_DIR)/prueba_vecindario
	$(BUILD_DIR)/schelling_seq --config tests/data/minima.conf
	OMP_NUM_THREADS=2 $(MPIEXEC) -np 2 $(BUILD_DIR)/schelling_hybrid \
		--config tests/data/minima.conf | tee $(BUILD_DIR)/salida_hibrida.txt
	grep -q "procesos mpi 2 threads por proceso 2" $(BUILD_DIR)/salida_hibrida.txt

format:
	clang-format -i $$(find app include src tests -type f \( -name '*.c' -o -name '*.h' \))

format-check:
	clang-format --dry-run --Werror $$(find app include src tests -type f \( -name '*.c' -o -name '*.h' \))

analyze:
	cppcheck --enable=warning,performance,portability --error-exitcode=1 \
		--std=c11 --language=c --suppress=missingIncludeSystem app include src tests

clean:
	rm -rf $(BUILD_DIR)
