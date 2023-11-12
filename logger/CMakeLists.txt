add_library(log SHARED
    boost_logger.cpp
)

target_link_libraries(log CONAN_PKG::boost)
