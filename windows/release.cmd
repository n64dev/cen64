@ECHO OFF
TITLE "CEN64 Release Build System"

ECHO Please ensure that you have a recent mingw64 installation available on
ECHO your system, and that you have included the bin\ folder of mingw64 in
ECHO your system path.
ECHO.
ECHO Please also make sure that libglfw.a and include\GL\glfw.h are located in
ECHO the root directory of CEN64. Note that you must have use the GLFW2 library,
ECHO not the GLFW3 library.
ECHO.
mingw32-make -s -C .. OS=windows
move /Y 1>NUL ..\cen64-ready.exe cen64.exe
del ..\cen64.exe

