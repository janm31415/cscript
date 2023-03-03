set(HDRS
compiler.h
constant.h
context.h
cscript.h
dump.h
environment.h
error.h
func.h
limits.h
map.h
memory.h
object.h
parser.h
pool.h
primitives.h
stream.h
string.h
syscalls.h
token.h
vector.h
visitor.h
vm.h
)
	
set(SRCS
compiler.c
constant.c
context.c
dump.c
environment.c
error.c
func.c
map.c
memory.c
object.c
parser.c
pool.c
primitives.c
stream.c
string.c
syscalls.c
token.c
visitor.c
vm.c
)

if (WIN32)
set(CMAKE_C_FLAGS_DEBUG "/W4 /MP /GF /RTCu /Od /MDd /Zi")
set(CMAKE_C_FLAGS_RELEASE "/W4 /MP /GF /O2 /Ob2 /Oi /Ot /MD /Zi /DNDEBUG")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif(WIN32)



add_library(cscript STATIC ${HDRS} ${SRCS})
source_group("Header Files" FILES ${hdrs})
source_group("Source Files" FILES ${srcs})

target_include_directories(cscript
    PRIVATE
	  ${CMAKE_CURRENT_SOURCE_DIR}/..
    )	
	
if (UNIX)
target_link_libraries(cscript
    PRIVATE	
    m	  
    )
endif (UNIX)    