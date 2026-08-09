cmake_minimum_required(VERSION 3.16)

project(hpc_schelling VERSION 0.1.0 LANGUAGES C)

option(HABILITAR_SANITIZADORES "habilitar address sanitizer y undefined behavior sanitizer" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_library(schelling_comun STATIC
    src/common/argumentos.c
    src/common/configuracion.c
    src/common/registro.c
)

target_include_directories(schelling_comun PUBLIC include)
target_compile_definitions(schelling_comun PUBLIC SCHELLING_VERSION="${PROJECT_VERSION}")
target_compile_options(schelling_comun PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

if(HABILITAR_SANITIZADORES)
    target_compile_options(schelling_comun PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(schelling_comun PUBLIC -fsanitize=address,undefined)
endif()

add_executable(schelling_seq app/schelling_seq.c)
target_link_libraries(schelling_seq PRIVATE schelling_comun)
target_compile_options(schelling_seq PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

find_package(MPI REQUIRED COMPONENTS C)
find_package(OpenMP REQUIRED COMPONENTS C)

add_executable(schelling_hybrid app/schelling_hybrid.c)
target_link_libraries(schelling_hybrid PRIVATE schelling_comun MPI::MPI_C OpenMP::OpenMP_C)
target_compile_options(schelling_hybrid PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

include(CTest)

if(BUILD_TESTING)
    add_executable(prueba_configuracion tests/unit/prueba_configuracion.c)
    target_link_libraries(prueba_configuracion PRIVATE schelling_comun)
    target_compile_options(prueba_configuracion PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    target_compile_definitions(prueba_configuracion PRIVATE
        RUTA_CONFIGURACION_PRUEBA="${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
    )
    add_test(NAME configuracion COMMAND prueba_configuracion)

    add_test(NAME inicio_secuencial
        COMMAND schelling_seq --config "${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
    )

    add_test(NAME argumento_invalido COMMAND schelling_seq --opcion-inexistente)
    set_tests_properties(argumento_invalido PROPERTIES WILL_FAIL TRUE)

    add_test(NAME configuracion_inexistente
        COMMAND schelling_seq --config "${CMAKE_SOURCE_DIR}/tests/data/no_existe.conf"
    )
    set_tests_properties(configuracion_inexistente PROPERTIES WILL_FAIL TRUE)

    add_test(NAME inicio_hibrido
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2
                $<TARGET_FILE:schelling_hybrid>
                --config "${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
    )
    set_tests_properties(inicio_hibrido PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=2"
        PASS_REGULAR_EXPRESSION "procesos mpi 2 threads por proceso 2"
        PROCESSORS 4
    )
endif()
