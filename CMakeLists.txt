cmake_minimum_required(VERSION 3.16)

set(debug false CACHE BOOL "")

project(LockFreeSet)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/..)
set(CMAKE_CXX_COMPILER /usr/bin/clang++)

set(CMAKE_CXX_STANDARD 20)


if (debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -fsanitize=thread -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
endif()

set(SOURCE_PATH tests)

set(SOURCES
	${SOURCE_PATH}/smoke_test.cpp
	)

include_directories(${CMAKE_SOURCE_DIR}/libcds)
link_directories(${CMAKE_SOURCE_DIR}/libcds/build/bin)

add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME} cds pthread gtest gtest_main)
