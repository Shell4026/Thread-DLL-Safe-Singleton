file(GLOB_RECURSE SRC CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(ShellSingleton STATIC ${SRC})

target_include_directories(ShellSingleton PRIVATE ${CMAKE_SOURCE_DIR}/include)