cmake_minimum_required(VERSION 3.16)

project(rbuflogd LANGUAGES C)

include(GNUInstallDirs)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_library(RT_LIBRARY rt)

set(RBUFLOGD_PUBLIC_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)

add_library(rbuflogd_common STATIC
  src/common/shmem.c
  src/common/ring_buffer.c
)

target_include_directories(rbuflogd_common
  PUBLIC
    ${RBUFLOGD_PUBLIC_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rbuflogd_common PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

if(RT_LIBRARY)
  target_link_libraries(rbuflogd_common PRIVATE ${RT_LIBRARY})
endif()

add_library(rbuflogd_producer SHARED
  src/producer/producer.c
)

target_include_directories(rbuflogd_producer
  PUBLIC
    ${RBUFLOGD_PUBLIC_INCLUDE_DIR}
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rbuflogd_producer PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

if(RT_LIBRARY)
  target_link_libraries(rbuflogd_producer PRIVATE ${RT_LIBRARY})
endif()

target_link_libraries(rbuflogd_producer PRIVATE rbuflogd_common)

add_executable(rbuflogd
  src/daemon/main.c
  src/daemon/consumer.c
  src/daemon/log_sink.c
  src/daemon/cli_parser.c
)

target_include_directories(rbuflogd
  PRIVATE
    ${RBUFLOGD_PUBLIC_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src/daemon
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rbuflogd PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

target_link_libraries(rbuflogd PRIVATE rbuflogd_common)

add_executable(rbuflogd_producer_test
  test/producer_smoketest.c
)

target_include_directories(rbuflogd_producer_test
  PRIVATE
    ${RBUFLOGD_PUBLIC_INCLUDE_DIR}
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rbuflogd_producer_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

target_link_libraries(rbuflogd_producer_test PRIVATE rbuflogd_producer)

set_target_properties(rbuflogd_producer_test PROPERTIES
  BUILD_RPATH "$ORIGIN"
)

install(TARGETS rbuflogd_producer
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES include/rbuflogd/producer.h
              include/rbuflogd/pub_common_types.h
              include/rbuflogd/logger.h
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/rbuflogd
)
