SET(MAIN_SRC
        main.cxx
        global.cxx
        )

SET(FETCH_SRC
        fetch/site.cxx
        fetch/sequencer.cxx
        fetch/hash_table.cxx
        fetch/checker.cxx
        fetch/file.cxx
        fetch/fetch_open.cxx
        fetch/fetch_pipe.cxx
        fetch/save_specific_buffer.cxx
        )

SET(IO_SRC
        io/input.cxx
        io/user_output.cxx
        io/output.cxx
        io/default_user_output.cxx
        io/save_user_output.cxx
        io/mirror_user_output.cxx
        io/stats_user_output.cxx
        )

SET(UTILS_SRC
        utils/string.cxx
        utils/debug.cxx
        utils/url.cxx
        utils/connection.cxx
        utils/text.cxx
        utils/histogram.cxx
        utils/webserver.cxx
        utils/persistent_fifo.cxx
        utils/hash_duplicate.cxx
        utils/thread.cxx
        utils/limit_time.cxx
        utils/limit_page.cxx
        utils/level.cxx
        utils/punycode.cxx
        )

SET(BASEDIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(ADNSDIR ${BASEDIR}/../adns/)

SET(CMAKE_CXX_FLAGS
        "-Wall -Wno-deprecated -O3 -D__STDC_LIMIT_MACROS -D_REENTRANT -I${BASEDIR} -I${ADNSDIR}"
        )

ADD_EXECUTABLE(
        larbin
        ${MAIN_SRC}
        ${FETCH_SRC}
        ${IO_SRC}
        ${UTILS_SRC}
        )

TARGET_LINK_LIBRARIES(
        larbin
        pthread
        resolv
        adns
        )

SET_TARGET_PROPERTIES(
        larbin
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        )
