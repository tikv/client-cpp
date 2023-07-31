project(Example)
cmake_minimum_required(VERSION 3.5)
set (CMAKE_CXX_STANDARD 17)

# using remote repository
# include(ExternalProject)
# set(EXTERNAL_INSTALL_LOCATION ${CMAKE_BINARY_DIR}/external)
# ExternalProject_Add(tikv
#     GIT_REPOSITORY https://github.com/Smityz/client-cpp.git
#     GIT_TAG cmake
#     CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${EXTERNAL_INSTALL_LOCATION}
# )
# include_directories(${EXTERNAL_INSTALL_LOCATION}/include)
# link_directories(${EXTERNAL_INSTALL_LOCATION}/lib)

# using local repository
find_package(tikvcpp 0.1.0)

add_executable(raw raw.cpp)
target_link_libraries(raw tikvcpp tikvrust pthread dl ssl crypto)
add_executable(txn txn.cpp)
target_link_libraries(txn tikvcpp tikvrust pthread dl ssl crypto)
