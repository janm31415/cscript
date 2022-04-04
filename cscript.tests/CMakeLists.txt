set(HDRS
compiletests.h
optimizetests.h
parsetests.h
test_assert.h
tokenizetests.h
    )
	
set(SRCS
compiletests.cpp
optimizetests.cpp
parsetests.cpp
test.cpp
test_assert.cpp
tokenizetests.cpp
)

if (WIN32)
set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
endif(WIN32)

# general build definitions
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

add_executable(cscript.tests ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(cscript.tests
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/..
    )
	
target_link_libraries(cscript.tests
    PRIVATE
    vm
    cscript
    )	