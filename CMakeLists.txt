cmake_minimum_required(VERSION 2.8)

PROJECT(LARBIN C CXX)

SET(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(BIN_DIR ${CMAKE_CURRENT_BINARY_DIR})

ADD_SUBDIRECTORY(adns)
ADD_SUBDIRECTORY(src)

configure_file(${SRC_DIR}/larbin.conf ${BIN_DIR}/ COPYONLY)
configure_file(${SRC_DIR}/larbin-cn.conf ${BIN_DIR}/ COPYONLY)
configure_file(${SRC_DIR}/larbin-test.conf ${BIN_DIR}/ COPYONLY)

ENABLE_TESTING()
ADD_TEST(larbin_project larbin -c larbin-test.conf)
