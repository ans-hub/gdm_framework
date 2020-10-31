@echo off

mkdir build

if not defined DevEnvDir (
    call "vcvarsall.bat" x64
)

set CF=/Od /Zi /EHsc
set SRC_FILES=snake.cc
set OBJS=/Fo.\build\
set PBOS=/Fd.\build\
set OUT=/OUT:build/snake_gl.exe
set UseEnv=true

set INC_1=/I "_libs/glm/"
set INC_2=/I "_libs/freeglut/include/"
set INC_3=/I "_libs/glew-2.1.0/include/"
set LIB_1=/LIBPATH:"_libs/freeglut/lib/x64/"
set LIB_2=/LIBPATH:"_libs/glew-2.1.0/lib/Release/x64/"

cl %CF% %OBJS% %PBOS% %SRC_FILES% %INC_1% %INC_2% %INC_3% /link %LIB_1% %LIB_2% freeglut.lib glew32.lib %OUT% 
