set(HDRS
compiler_tests.h
dump_tests.h
map_tests.h
parser_tests.h
stream_tests.h
string_tests.h
test_assert.h
token_tests.h
vector_tests.h
)
	
set(SRCS
compiler_tests.c
dump_tests.c
map_tests.c
parser_tests.c
stream_tests.c
string_tests.c
test.c
test_assert.c
token_tests.c
vector_tests.c
)

if (WIN32)
set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
endif(WIN32)

add_definitions(-DMEMORY_LEAK_TRACKING)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)

add_executable(cscript.tests ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(cscript.tests
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/..
    )	
	
target_link_libraries(cscript.tests
    PRIVATE		 
    cscript
    )