cmake_minimum_required(VERSION 3.16)

project(hpc_schelling VERSION 0.1.0 LANGUAGES C)

option(HABILITAR_SANITIZADORES "habilitar address sanitizer y undefined behavior sanitizer" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

add_library(schelling_comun STATIC
    src/common/aleatorio.c
    src/common/argumentos.c
    src/common/checkpoint.c
    src/common/configuracion.c
    src/common/economia.c
    src/common/generador.c
    src/common/hash.c
    src/common/indice_vacantes.c
    src/common/modelo.c
    src/common/particion.c
    src/common/registro.c
    src/common/salida.c
    src/common/vecindario.c
    src/sequential/simulacion.c
)

target_include_directories(schelling_comun PUBLIC include)
target_link_libraries(schelling_comun PUBLIC m)
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
target_sources(schelling_hybrid PRIVATE src/parallel/simulacion_mpi.c)
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

    add_executable(prueba_checkpoint tests/unit/prueba_checkpoint.c)
    target_link_libraries(prueba_checkpoint PRIVATE schelling_comun)
    target_compile_options(prueba_checkpoint PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                        -Wshadow)
    add_test(NAME checkpoint COMMAND prueba_checkpoint)

    add_executable(prueba_aleatorio tests/unit/prueba_aleatorio.c)
    target_link_libraries(prueba_aleatorio PRIVATE schelling_comun)
    target_compile_options(prueba_aleatorio PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                      -Wshadow)
    add_test(NAME aleatorio COMMAND prueba_aleatorio)

    add_executable(prueba_generador tests/unit/prueba_generador.c)
    target_link_libraries(prueba_generador PRIVATE schelling_comun)
    target_compile_options(prueba_generador PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                      -Wshadow)
    add_test(NAME generador COMMAND prueba_generador)

    add_executable(prueba_hash tests/unit/prueba_hash.c)
    target_link_libraries(prueba_hash PRIVATE schelling_comun)
    target_compile_options(prueba_hash PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    add_test(NAME hash COMMAND prueba_hash)

    add_executable(prueba_economia tests/unit/prueba_economia.c)
    target_link_libraries(prueba_economia PRIVATE schelling_comun)
    target_compile_options(prueba_economia PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                     -Wshadow)
    add_test(NAME economia COMMAND prueba_economia)

    add_executable(prueba_simulacion tests/unit/prueba_simulacion.c)
    target_link_libraries(prueba_simulacion PRIVATE schelling_comun)
    target_compile_options(prueba_simulacion PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                        -Wshadow)
    add_test(NAME simulacion COMMAND prueba_simulacion)

    add_executable(prueba_modelo tests/unit/prueba_modelo.c)
    target_link_libraries(prueba_modelo PRIVATE schelling_comun)
    target_compile_options(prueba_modelo PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
    add_test(NAME modelo COMMAND prueba_modelo)

    add_executable(prueba_particion tests/unit/prueba_particion.c)
    target_link_libraries(prueba_particion PRIVATE schelling_comun)
    target_compile_options(prueba_particion PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                     -Wshadow)
    add_test(NAME particion COMMAND prueba_particion)

    add_executable(prueba_indice_vacantes tests/unit/prueba_indice_vacantes.c)
    target_link_libraries(prueba_indice_vacantes PRIVATE schelling_comun)
    target_compile_options(prueba_indice_vacantes PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                           -Wshadow)
    add_test(NAME indice_vacantes COMMAND prueba_indice_vacantes)

    add_executable(prueba_vecindario tests/unit/prueba_vecindario.c)
    target_link_libraries(prueba_vecindario PRIVATE schelling_comun)
    target_compile_options(prueba_vecindario PRIVATE -Wall -Wextra -Wpedantic -Wconversion
                                                       -Wshadow)
    add_test(NAME vecindario COMMAND prueba_vecindario)

    add_test(NAME inicio_secuencial
        COMMAND schelling_seq --config "${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
    )
    set_tests_properties(inicio_secuencial PROPERTIES
        PASS_REGULAR_EXPRESSION "hash 7d65e8c90d52123b"
    )

    add_test(NAME argumento_invalido COMMAND schelling_seq --opcion-inexistente)
    set_tests_properties(argumento_invalido PROPERTIES WILL_FAIL TRUE)

    add_test(NAME configuracion_inexistente
        COMMAND schelling_seq --config "${CMAKE_SOURCE_DIR}/tests/data/no_existe.conf"
    )
    set_tests_properties(configuracion_inexistente PROPERTIES WILL_FAIL TRUE)

    add_test(NAME mpi_un_proceso
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 1
                $<TARGET_FILE:schelling_hybrid>
                --config "${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
                --output "${CMAKE_BINARY_DIR}/salida_mpi_1"
    )
    set_tests_properties(mpi_un_proceso PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=1"
        PASS_REGULAR_EXPRESSION "hash 7d65e8c90d52123b"
        PROCESSORS 1
    )

    add_test(NAME mpi_dos_procesos
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 2
                $<TARGET_FILE:schelling_hybrid>
                --config "${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
                --output "${CMAKE_BINARY_DIR}/salida_mpi_2"
    )
    set_tests_properties(mpi_dos_procesos PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=1"
        PASS_REGULAR_EXPRESSION "hash 7d65e8c90d52123b"
        PROCESSORS 2
    )

    add_test(NAME mpi_tres_procesos
        COMMAND ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} 3
                $<TARGET_FILE:schelling_hybrid>
                --config "${CMAKE_SOURCE_DIR}/tests/data/minima.conf"
                --output "${CMAKE_BINARY_DIR}/salida_mpi_3"
    )
    set_tests_properties(mpi_tres_procesos PROPERTIES
        ENVIRONMENT "OMP_NUM_THREADS=1"
        PASS_REGULAR_EXPRESSION "hash 7d65e8c90d52123b"
        PROCESSORS 3
    )
endif()
