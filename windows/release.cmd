@ECHO OFF
TITLE "CEN64 Release Build System"

ECHO Please ensure that you have a recent mingw64 installation available on
ECHO your system, and that you have included the bin\ folder of mingw64 in
ECHO your system path.
ECHO.
ECHO Please also make sure that the glfw2 libraries and dlls are located in
ECHO the root directory of CEN64, and that the GL\ folder containing the
ECHO glfw.h header is located in a directory named 'include' inside the CEN64
ECHO root directory. You may also find that you need OpenGL32.Lib from the
ECHO Microsoft SDK in the root directory.
ECHO.
mingw32-make -s -C .. OS=windows
copy /Y 1>NUL ..\cen64.exe .

