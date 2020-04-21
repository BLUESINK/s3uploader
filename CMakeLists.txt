cmake_minimum_required (VERSION 2.6)

project (s3_uploader)

include_directories(include)

file(GLOB SOURCES_AWS "aws/*.c")
file(GLOB SOURCES_CRYPTO "crypto/*.c")

add_executable(s3_uploader main.c ${SOURCES_AWS} ${SOURCES_CRYPTO})
