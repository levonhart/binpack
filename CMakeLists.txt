cmake_minimum_required(VERSION 3.15...3.19)

project(BinPacking VERSION 0.1.0 LANGUAGES C
		DESCRIPTION "Bin Packing Problem heuristics implementations.")

add_library(binpack OBJECT)
target_sources( binpack
				PRIVATE "binpack.c" "binpack-solution.c" "binpack-search.c"
				INTERFACE "binpack.h")

add_subdirectory(tests)

option(MSVC_DLL "Dynamic link C runtime library (Windows Only)" OFF)
option(TEST_ON "Generate Ctest configuration files" ON)

target_include_directories( binpack PUBLIC "${BinPacking_SOURCE_DIR}")

add_executable(benchmark)
target_sources( benchmark
				PRIVATE "benchmark.c")
target_link_libraries( benchmark PUBLIC binpack)



add_test(readfile "${BinPacking_SOURCE_DIR}/build/tests/readfile" "${BinPacking_SOURCE_DIR}/tests/instances/Scholl/Scholl_1/N1C1W1_A.txt")
add_test(solution "${BinPacking_SOURCE_DIR}/build/tests/solution")
add_test(search "${BinPacking_SOURCE_DIR}/build/tests/solution")
if(TEST_ON)
	enable_testing()
endif()
