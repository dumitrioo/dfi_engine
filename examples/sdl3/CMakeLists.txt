cmake_minimum_required(VERSION 3.22)

add_library(dfisdl SHARED
    libmain.cpp
)

target_link_libraries(dfisdl PRIVATE
    SDL3
)

if(UNIX AND NOT APPLE)
    target_link_libraries(dfisdl PRIVATE pthread dl)
endif()
