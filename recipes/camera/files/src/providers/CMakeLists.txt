
find_package(Vmb REQUIRED COMPONENTS CPP)

add_library(libproviders STATIC vimba_provider.cpp message_queue.cpp csp_server.cpp csp_server_posix.cpp)
target_include_directories(libproviders PRIVATE ${CMAKE_SOURCE_DIR}/src/include/providers ${CMAKE_SOURCE_DIR}/src/include ${CMAKE_SOURCE_DIR}/src/include/helpers)

target_link_libraries(libproviders PRIVATE Vmb::CPP libhelpers)