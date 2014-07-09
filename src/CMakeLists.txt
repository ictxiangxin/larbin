SET(MAIN_SRC
        main.cc
        global.cc
        )

SET(FETCH_SRC
        fetch/site.cc
        fetch/sequencer.cc
        fetch/hashTable.cc
        fetch/checker.cc
        fetch/file.cc
        fetch/fetchOpen.cc
        fetch/fetchPipe.cc
        fetch/savespecbuf.cc
        )

SET(INTERF_SRC
        interf/input.cc
        interf/useroutput.cc
        interf/output.cc
        interf/defaultuseroutput.cc
        interf/saveuseroutput.cc
        interf/mirrorsaveuseroutput.cc
        interf/statsuseroutput.cc
        )

SET(UTILS_SRC
        utils/string.cc
        utils/debug.cc
        utils/url.cc
        utils/connexion.cc
        utils/text.cc
        utils/histogram.cc
        utils/webserver.cc
        utils/PersistentFifo.cc
        utils/hashDup.cc
        utils/mypthread.cc
        utils/limitTime.cc
        utils/level.cc
        utils/punycode.cc
        )

SET(BASEDIR ${CMAKE_CURRENT_SOURCE_DIR})
SET(ADNSDIR ${BASEDIR}/../adns/)

SET(CMAKE_CXX_FLAGS
        "-Wall -Wno-deprecated -O3 -D__STDC_LIMIT_MACROS -D_REENTRANT -I${BASEDIR} -I${ADNSDIR}"
        )

add_executable(
        larbin
        ${MAIN_SRC}
        ${FETCH_SRC}
        ${INTERF_SRC}
        ${UTILS_SRC}
        )

TARGET_LINK_LIBRARIES(
        larbin
        pthread
        resolv
        nsl
        adns
        )

SET_TARGET_PROPERTIES(
        larbin
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BIN_DIR}
        )
