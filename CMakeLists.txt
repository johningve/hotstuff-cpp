project("hotstuff")

cmake_minimum_required(VERSION 3.21)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_path(BOTAN_INCLUDE_DIR botan/botan.h REQUIRED)
# include_directories(${BOTAN_INCLUD})
find_library(BOTAN_LIBRARY botan-2 REQUIRED)

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/lib")

# asio provides CMake targets:

#     # this is heuristically generated, and may not be correct
#     find_package(asio CONFIG REQUIRED)
#     target_link_libraries(main PRIVATE asio::asio)

# cereal provides CMake targets:

    # this is heuristically generated, and may not be correct
find_package(cereal CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)

# catch2 provides CMake targets:

#     # this is heuristically generated, and may not be correct
#     find_package(Catch2 CONFIG REQUIRED)
#     target_link_libraries(main PRIVATE Catch2::Catch2 Catch2::Catch2WithMain)

# include_directories(lib/secp256k1/include)

# add_subdirectory(lib)
add_subdirectory(src)
