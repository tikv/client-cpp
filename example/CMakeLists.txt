project(Example)
cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)

# using local repository
find_package(tikvcpp 0.1.0)

add_executable(raw raw.cpp)
target_link_libraries(raw tikvcpp tikvrust pthread dl ssl crypto)
add_executable(txn txn.cpp)
target_link_libraries(txn tikvcpp tikvrust pthread dl ssl crypto)
