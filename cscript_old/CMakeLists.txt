set(HDRS
compiler.h
cscript.h
cscript_api.h
defines.h
namespace.h
optimize.h
parse.h
peephole.h
pp_visitor.h
regalloc.h
tokenize.h
utility.h
visitor.h
)
	
set(SRCS
compiler.cpp
cscript.cpp
optimize.cpp
parse.cpp
peephole.cpp
regalloc.cpp
utility.cpp
tokenize.cpp
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

add_library(cscript_old SHARED ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(cscript_old
    PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
    )	
	
target_link_libraries(cscript_old
    PRIVATE	
	  vm
    )	