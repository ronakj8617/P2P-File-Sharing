cmake_minimum_required(VERSION 3.16)
project(P2PFileSharing)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Common library
add_library(p2p_common
    src/common/FileChunk.cpp
    src/common/Protocol.cpp
    src/common/Utils.cpp
)

target_include_directories(p2p_common PUBLIC include)
target_link_libraries(p2p_common Threads::Threads)

# Tracker executable
add_executable(tracker
    src/tracker/main.cpp
    src/tracker/Tracker.cpp
)

target_include_directories(tracker PRIVATE include)
target_link_libraries(tracker p2p_common Threads::Threads)

# Peer executable
add_executable(peer
    src/peer/main.cpp
    src/peer/Peer.cpp
)

target_include_directories(peer PRIVATE include)
target_link_libraries(peer p2p_common Threads::Threads)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(p2p_common PRIVATE -Wall -Wextra -O2)
    target_compile_options(tracker PRIVATE -Wall -Wextra -O2)
    target_compile_options(peer PRIVATE -Wall -Wextra -O2)
endif()