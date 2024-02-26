find_package(OpenCV REQUIRED)

add_library(libhelpers STATIC exposure_helper.cpp csp_parser.cpp)
target_include_directories(libhelpers PRIVATE ${CMAKE_SOURCE_DIR}/src/include/helpers ${CMAKE_SOURCE_DIR}/src/include/providers ${CMAKE_SOURCE_DIR}/src/include)

target_link_libraries(libhelpers PRIVATE ${OpenCV_LIBS} Vmb::CPP libproviders)