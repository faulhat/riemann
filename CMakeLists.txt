cmake_minimum_required(VERSION 3.5)

Project(Riemann)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

find_program(LEX_EXE
    flex
)
if(LEX_EXE STREQUAL "LEX_EXE-NOTFOUND")
    message(FATAL_ERROR "dear user, plase install flex!")
endif(LEX_EXE STREQUAL "LEX_EXE-NOTFOUND")

find_program(YACC_EXE
    bison
)
if(YACC_EXE STREQUAL "YACC_EXE-NOTFOUND")
    message(FATAL_ERROR "dear user, plase install bison!")
endif(YACC_EXE STREQUAL "YACC_EXE-NOTFOUND")

# reuseable cmake macro for yacc
MACRO(YACC_FILE _filename)
    GET_FILENAME_COMPONENT(_basename ${_filename} NAME_WE)
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.c
                ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.h
        COMMAND ${YACC_EXE} -d
                -Wcex
                --output=${_basename}.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${_filename}
        DEPENDS ${_filename}
    )
ENDMACRO(YACC_FILE)

# reuseable cmake macro for lex
MACRO(LEX_FILE _filename)
    GET_FILENAME_COMPONENT(_basename ${_filename} NAME_WE)
    ADD_CUSTOM_COMMAND(
        OUTPUT  ${CMAKE_CURRENT_BINARY_DIR}/${_basename}.c
        COMMAND ${LEX_EXE}
                --header-file=${_basename}.h
                -o${_basename}.c
                ${CMAKE_CURRENT_SOURCE_DIR}/${_filename}
        DEPENDS ${_filename} )
ENDMACRO(LEX_FILE)

YACC_FILE(parser.y)
LEX_FILE(lexer.l)

# Tell asmjit that it will be embedded.
Set(ASMJIT_EMBED TRUE)
Add_Definitions(-DASMJIT_STATIC)

Set(ASMJIT_DIR "asmjit/")
Include("${ASMJIT_DIR}/CMakeLists.txt")

Set(CMAKE_C_FLAGS "$(CMAKE_C_FLAGS) -g")
Set(CMAKE_CXX_FLAGS "-g")

Add_Executable(rcalc
   parser.c
   lexer.c
   expr.c
   compile.cpp
   repl.cpp 
   test.cpp
   main.cpp
   graph.cpp
   ${ASMJIT_SRC})

Target_Link_Libraries(rcalc ${ASMJIT_DEPS} ${GTK3_LIBRARIES})
Include_Directories(${GTK3_INCLUDE_DIRS})
Link_Directories(${GTK3_LIBRARY_DIRS})
Add_Definitions(${GTK3_CFLAGS_OTHER})

