cmake_minimum_required(VERSION 3.16)

project(rbuflogd LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(rbuflogd
  rbuflogd_main.c
  rbuflogd_consumer.c
  rbuflogd_shmem.c
)

target_include_directories(rbuflogd
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rbuflogd PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

# shm_open can require librt on some Linux setups.
find_library(RT_LIBRARY rt)
if(RT_LIBRARY)
  target_link_libraries(rbuflogd PRIVATE ${RT_LIBRARY})
endif()

# --- Test producer application ---
add_executable(rbuflogd_producer_test
  test/rbuflogd_producer_test.c
)

target_include_directories(rbuflogd_producer_test
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(rbuflogd_producer_test PRIVATE
    -Wall
    -Wextra
    -Wpedantic
  )
endif()

if(RT_LIBRARY)
  target_link_libraries(rbuflogd_producer_test PRIVATE ${RT_LIBRARY})
endif()
