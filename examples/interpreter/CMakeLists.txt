cmake_minimum_required(VERSION 3.22)

add_definitions(-DDFI_USE_EIGEN)

add_definitions(-DUSE_CUSTOM_MEMORY_ALLOCATION)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
    ../../src
)

add_executable(dfi
    main.cpp
)

if(UNIX AND NOT APPLE)
    target_link_libraries(dfi
        pthread
        dl
    )
endif()
