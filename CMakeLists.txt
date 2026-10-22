cmake_minimum_required(VERSION 3.21)
project(hft_lab VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(HFT_SANITIZERS "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(HFT_SANITIZERS)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(FATAL_ERROR "The sanitizer preset requires Clang")
  endif()
  add_compile_options(-fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer)
  add_link_options(-fsanitize=address,undefined)
endif()
add_compile_options(-Wall -Wextra -Wpedantic)

add_library(hft_core src/price_level.cpp src/order_book.cpp)
target_include_directories(hft_core PUBLIC include)

add_executable(hft_lab src/main.cpp)
target_link_libraries(hft_lab PRIVATE hft_core)

enable_testing()
add_test(NAME cli_contract COMMAND ${CMAKE_COMMAND}
  -DAPP=$<TARGET_FILE:hft_lab> -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/cli_contract.cmake)
set_tests_properties(cli_contract PROPERTIES LABELS scaffold)

add_executable(price_level_tests tests/price_level_tests.cpp)
target_link_libraries(price_level_tests PRIVATE hft_core)
add_test(NAME price_level COMMAND price_level_tests)
set_tests_properties(price_level PROPERTIES LABELS exercise)
