set(HDRS
alpha.h
compiler.h
constant.h
constfold.h
constprop.h
context.h
cscript.h
dump.h
environment.h
error.h
foreign.h
func.h
limits.h
map.h
memory.h
object.h
parser.h
preprocess.h
primitives.h
remdeadvar.h
stream.h
string.h
syscalls.h
token.h
vector.h
visitor.h
vm.h
)
	
set(SRCS
alpha.c
compiler.c
constant.c
constfold.c
constprop.c
context.c
dump.c
environment.c
error.c
foreign.c
func.c
map.c
memory.c
object.c
parser.c
preprocess.c
primitives.c
remdeadvar.c
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