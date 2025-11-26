cmake_minimum_required(VERSION 3.15)
project(PSDThumbnailProvider)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output DLL to build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(PSDThumbnailProvider SHARED
    src/PSDThumbnailProvider.cpp
    src/PSDParser.cpp
    src/ClassFactory.cpp
    src/DllMain.cpp
    src/PSDThumbnailProvider.def
)

target_include_directories(PSDThumbnailProvider PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(PSDThumbnailProvider
    shlwapi
    gdiplus
    ole32
    oleaut32
    uuid
)

# Generate GUID if needed
add_definitions(-DUNICODE -D_UNICODE)

set_target_properties(PSDThumbnailProvider PROPERTIES
    PREFIX ""
    SUFFIX ".dll"
)
