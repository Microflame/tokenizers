cmake_minimum_required(VERSION 3.5)
project(byte-level-bpe)
set(CMAKE_CXX_STANDARD 17)


find_package(absl)


add_executable(main main.cpp)


add_library(byte-level-bpe byte-level-bpe.cpp)
target_include_directories(byte-level-bpe PUBLIC "~/install/include")
if (absl_FOUND)
    message(STATUS "Building with Abseil!")
    target_compile_definitions(byte-level-bpe PUBLIC TOKENIZERS_ABSL_ENABLED=1)
    set(ABSL_FLAT_HASH_MAP absl::flat_hash_map)
else(absl_FOUND)
set(ABSL_FLAT_HASH_MAP "")
endif (absl_FOUND)

target_link_libraries(main
    byte-level-bpe
    ${ABSL_FLAT_HASH_MAP}
)