include_directories(${CMAKE_SOURCE_DIR}/logger)

add_executable(monitor main.cpp base_interface.cpp)

target_link_libraries(monitor CONAN_PKG::boost log)
