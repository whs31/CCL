cmake_minimum_required (VERSION 3.15)

project(CCL LANGUAGES CXX)

if(TARGET ${PROJECT_NAME})
    return()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt5 Qt6 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS 
    Core
    Positioning
)

include_directories(src include)

file(GLOB_RECURSE CCL_CORE src/*)
file(GLOB_RECURSE CCL_CORE_INCLUDE include/*)

set(SRC ${CCL_CORE} ${CCL_CORE_INCLUDE})

add_library(${PROJECT_NAME} ${SRC})

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Positioning
)

<<<<<<< Updated upstream
=======
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

target_compile_definitions(${PROJECT_NAME} PUBLIC
  -DM_PI=3.14159265358979323846
)

>>>>>>> Stashed changes
target_include_directories(${PROJECT_NAME} PUBLIC include src)