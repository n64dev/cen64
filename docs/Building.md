# Build requirements

* CMake
* iconv
* OpenAL
* OpenGL

To build on Fedora 29, do: `sudo dnf install cmake make mesa-libGL-devel openal-soft-devel`

To build for Windows on Fedora 29, do: `sudo dnf install cmake make mingw64-{gcc,iconv,openal}`

To build for Windows on Windows XP..10, do:
1. Install MSYS 1.0.11: https://sourceforge.net/projects/mingw/files/MSYS/Base/msys-core/msys-1.0.11/MSYS-1.0.11.exe/download
* Say yes to post install
* Say no to mingw is installed
* Press enter a few times to finish the install
2. Extract contents of "mingw64" directory to "MSYS\mingw" directory: https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z
3. Copy "MSYS\mingw\bin\mingw32-make.exe" to "MSYS\mingw\bin\make.exe"
4. Extract OpenAL "bin, libs, include" directories to "MSYS\mingw\x86_64-w64-mingw32" directory: https://openal-soft.org/openal-binaries/openal-soft-1.21.0-bin.zip
5. Extract iconv to "MSYS\home\yourname\libiconv" directory: https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz
6. Run MSYS & type "cd libiconv" Enter, followed by "./configure --disable-shared" Enter, followed by "make install" Enter
7. Extract CMake "bin, doc, man, share" directories to "MSYS" directory: https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4-win64-x64.zip
8. Extract CEN64 source to "MSYS\home\yourname\cen64" directory: https://github.com/n64dev/cen64
9. Run MSYS & type "cd cen64" Enter, followed by "cmake-gui" Enter
10. Add the "MSYS/home/yourname/cen64" directory to "Browse Source..." & "Browse Build...", then Generate MSYS compatible files:
* Add OpenAL "MSYS/mingw/x86_64-w64-mingw32/include/AL" as "OPENAL/OPENAL_INCLUDE_DIR" & "MSYS/mingw/x86_64-w64-mingw32/lib/libOpenAL32.dll.a" as "OPENAL/OPENAL_LIBRARY" (if they are not found)
* Make sure to use libiconv static lib (Not DLL) "MSYS/local/lib/libiconv.a" as "ICONV/ICONV_LIBRARIES"
* Select "SSE4.1" or "AVX" as "Ungrouped Entries/CEN64_ARCH_SUPPORT" & make sure VR4300_BUSY_WAIT_DETECTION is ticked
11. Click "Configure" then click "Generate" to get the makefiles, then quit cmake-gui, & type "make" Enter
