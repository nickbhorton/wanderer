cmake_minimum_required(VERSION 3.29)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED On)

project(
    wanderer
    LANGUAGES CXX
)

add_library(
    udp_server 
    lib/udp_server.cc
)

add_library(
    garmin_sdk
    ext/cpp/fit_accumulated_field.cpp
    ext/cpp/fit_accumulator.cpp
    ext/cpp/fit_buffered_mesg_broadcaster.cpp
    ext/cpp/fit_buffered_record_mesg_broadcaster.cpp
    ext/cpp/fit_buffer_encode.cpp
    ext/cpp/fit.cpp
    ext/cpp/fit_crc.cpp
    ext/cpp/fit_date_time.cpp
    ext/cpp/fit_decode.cpp
    ext/cpp/fit_developer_field.cpp
    ext/cpp/fit_developer_field_definition.cpp
    ext/cpp/fit_developer_field_description.cpp
    ext/cpp/fit_encode.cpp
    ext/cpp/fit_factory.cpp
    ext/cpp/fit_field_base.cpp
    ext/cpp/fit_field.cpp
    ext/cpp/fit_field_definition.cpp
    ext/cpp/fit_mesg_broadcaster.cpp
    ext/cpp/fit_mesg.cpp
    ext/cpp/fit_mesg_definition.cpp
    ext/cpp/fit_mesg_with_event_broadcaster.cpp
    ext/cpp/fit_profile.cpp
    ext/cpp/fit_protocol_validator.cpp
    ext/cpp/fit_unicode.cpp
)

add_executable(
    fit_decode_example
    ext/cpp/examples/decode/decode.cpp
)
target_include_directories(
    fit_decode_example
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ext/cpp
)
target_link_libraries(
    fit_decode_example
    garmin_sdk
)

add_library(
    fit_utils
    src/fit_defaults.cc
)
target_include_directories(
    fit_utils
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ext/cpp
)
target_link_libraries(
    fit_utils
    garmin_sdk
)

add_executable(
    fit_types
    src/fit_types.cc
)
target_include_directories(
    fit_types
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ext/cpp
)
target_link_libraries(
    fit_types
    fit_utils
    garmin_sdk
)

add_executable(
    fit_coords
    src/fit_coords.cc
)
target_include_directories(
    fit_coords
    PUBLIC
    ${CMAKE_SOURCE_DIR}/ext/cpp
)
target_link_libraries(
    fit_coords
    fit_utils
    garmin_sdk
)
