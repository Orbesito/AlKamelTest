cmake_minimum_required(VERSION 3.21)

project(AlkamelTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)

set(ALKAMEL_SOURCES
    src/main.cpp
    src/app/MainWindow.cpp
    src/network/AlkamelClient.cpp
    src/network/AppConfig.cpp
    src/network/AlkamelSession.cpp
    src/network/ProtocolMessage.cpp
    src/network/ProtocolParser.cpp
    src/network/ProtocolSerializer.cpp
    src/state/JsonStateStore.cpp
)

set(ALKAMEL_HEADERS
    src/app/MainWindow.h
    src/network/AlkamelClient.h
    src/network/AppConfig.h
    src/network/AlkamelSession.h
    src/network/ProtocolMessage.h
    src/network/ProtocolParser.h
    src/network/ProtocolSerializer.h
    src/state/JsonStateStore.h
)

add_executable(alkamel_client
    ${ALKAMEL_SOURCES}
    ${ALKAMEL_HEADERS}
)

target_include_directories(alkamel_client PRIVATE src)

target_link_libraries(alkamel_client
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Network
        Qt${QT_VERSION_MAJOR}::Widgets
)

if(MSVC)
    target_compile_options(alkamel_client PRIVATE /W4 /permissive-)
else()
    target_compile_options(alkamel_client PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(alkamel_client)
endif()

enable_testing()

add_executable(json_state_store_tests
    tests/state/JsonStateStoreTests.cpp
    src/state/JsonStateStore.cpp
    src/state/JsonStateStore.h
)

target_include_directories(json_state_store_tests PRIVATE src)

target_link_libraries(json_state_store_tests
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Core
)

if(MSVC)
    target_compile_options(json_state_store_tests PRIVATE /W4 /permissive-)
else()
    target_compile_options(json_state_store_tests PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_test(NAME json_state_store_tests COMMAND json_state_store_tests)
