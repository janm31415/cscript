set(HDRS
compiler.h
cscript_api.h
namespace.h
optimize.h
parse.h
pp_visitor.h
tokenize.h
utility.h
visitor.h
)
	
set(SRCS
compiler.cpp
optimize.cpp
parse.cpp
utility.cpp
tokenize.cpp
)

set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_CXX_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")

# general build definitions
add_definitions(-D_UNICODE)
add_definitions(-DUNICODE)
add_definitions(-D_SCL_SECURE_NO_WARNINGS)

add_library(cscript SHARED ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(cscript
    PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/..
    )	
	
target_link_libraries(cscript
    PRIVATE	
	  vm
    )	