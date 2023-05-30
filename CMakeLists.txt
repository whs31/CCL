cmake_minimum_required (VERSION 3.15)

project(CCL LANGUAGES CXX)

set(CCL_COMPILE_TO_STATIC FALSE)
set(CCL_TESTS_ENABLED FALSE)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core
    Positioning
)
    
if(${LPVL_TESTS_ENABLED})
    find_package(GTest)
endif()

include_directories(src include)

set(CCL_CORE
    src/global/cclglobal.h
)

set(CCL_CORE_INCLUDE

)

set(SRC
    ${CCL_CORE} ${CCL_CORE_INCLUDE}
)

message("Compiling to static status: ${CCL_COMPILE_TO_STATIC}")
if(${CCL_COMPILE_TO_STATIC})
    add_library(${PROJECT_NAME} ${SRC})
    target_compile_definitions (${PROJECT_NAME} PRIVATE CCL_LIBRARY CCL_STATIC)
else()
    add_library(${PROJECT_NAME} SHARED ${SRC} )
    target_compile_definitions (${PROJECT_NAME} PRIVATE CCL_LIBRARY)
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Positioning
)

if(${CCL_TESTS_ENABLED})
    set(TESTS
        ${SRC}
    )

    add_executable(test_ccl ${TESTS})

    target_link_libraries(test_ccl
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Positioning
        GTest::GTest
    )
endif()

target_include_directories(${PROJECT_NAME} PUBLIC include src)

set_target_properties (${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "ccl"
    PREFIX "lib"
)
