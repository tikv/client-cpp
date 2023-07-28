cmake_minimum_required(VERSION 3.10)
cmake_policy(SET CMP0048 NEW)
project(tikvcpp VERSION 0.1.0)
set (CMAKE_CXX_STANDARD 17)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CARGO_CMD cargo build --verbose --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "debug")
else ()
    set(CARGO_CMD cargo build --release --target-dir=${CMAKE_CURRENT_BINARY_DIR})
    set(TARGET_DIR "release")
endif ()

set(RUST_PART_LIB "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_DIR}/libtikvrust.a")
set(RUST_PART_CXX "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/client-cpp/src/lib.rs.cc")
set(RUST_PART_H "${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/client-cpp/src/lib.rs.h")
add_custom_command(
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/Cargo.toml ${CMAKE_CURRENT_SOURCE_DIR}/src/lib.rs ${CMAKE_CURRENT_SOURCE_DIR}/build.rs
    OUTPUT ${RUST_PART_LIB} ${RUST_PART_CXX} ${RUST_PART_H}
    COMMAND CARGO_TARGET_DIR=${CMAKE_CURRENT_BINARY_DIR} RUSTFLAGS="${RUST_FLAGS}" ${CARGO_CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}/cxxbridge/client-cpp/src
    ${CMAKE_SOURCE_DIR}/include
)
add_library(tikvcpp STATIC src/tikv_client.cpp ${RUST_PART_CXX})
set_target_properties(tikvcpp PROPERTIES PUBLIC_HEADER "${CMAKE_SOURCE_DIR}/include/tikv_client.h;${RUST_PART_H}")

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    tikvcppConfigVersion.cmake
    VERSION ${PACKAGE_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(TARGETS tikvcpp 
        EXPORT tikvcppTargets 
        LIBRARY DESTINATION lib
        PUBLIC_HEADER DESTINATION include/tikv
        )

install(FILES ${RUST_PART_LIB} DESTINATION lib)

install(EXPORT tikvcppTargets
        FILE tikvcppTargets.cmake
        NAMESPACE tikvcpp::
        DESTINATION lib/cmake/tikvcpp
        )

configure_file(tikvcppConfig.cmake.in tikvcppConfig.cmake @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/tikvcppConfig.cmake"
                "${CMAKE_CURRENT_BINARY_DIR}/tikvcppConfigVersion.cmake"
        DESTINATION lib/cmake/tikvcpp
        )