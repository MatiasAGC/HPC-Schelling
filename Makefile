CC := gcc
MPICC := mpicc
MPIEXEC := mpirun
CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Iinclude
RELEASE_FLAGS := -O3 -DNDEBUG
DEBUG_FLAGS := -O0 -g3
OPENMP_FLAGS := -fopenmp
BUILD_DIR := build-make
COMMON_SOURCES := src/common/argumentos.c src/common/configuracion.c src/common/registro.c
COMMON_OBJECTS := $(COMMON_SOURCES:src/common/%.c=$(BUILD_DIR)/make/%.o)
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
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@

$(BUILD_DIR)/schelling_hybrid: app/schelling_hybrid.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(MPICC) $(CPPFLAGS) $(CFLAGS) $(OPENMP_FLAGS) $^ -o $@

$(BUILD_DIR)/make/%.o: src/common/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/prueba_configuracion: tests/unit/prueba_configuracion.c $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) \
		-DRUTA_CONFIGURACION_PRUEBA=\"tests/data/minima.conf\" $^ -o $@

test: debug $(BUILD_DIR)/prueba_configuracion
	$(BUILD_DIR)/prueba_configuracion
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
