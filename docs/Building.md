# Building

## Requirements

- `CMake`
- `iconv`
- `OpenAL`
- `OpenGL`

---

## Fedora 29

#### For Fedora

```sh
sudo dnf install cmake make mesa-libGL-devel openal-soft-devel
```

#### For Windows

```sh
sudo dnf install cmake make mingw64-{gcc,iconv,openal}
```

---

## Windows XP - 10

##### #1 Install **[MSYS]** `1.0.11`

- Select <kbd>Yes</kbd> for `Post Install`.

- Select <kbd>No</kbd> for `Mingw Is Installed`.

- Press <kbd>Enter</kbd> until finished.

<br>

##### #2 Extract **[MingW64]**

Place the content in `MSYS\mingw`.

<br>

##### #3 Copy

`MSYS\mingw\bin\mingw32-make.exe` to `MSYS\mingw\bin\make.exe`.

<br>

##### #4 Extract **[OpenAL]**

Place the `bin`, `libs` & `include` folders <br>
to `MSYS\mingw\x86_64-w64-mingw32\`.

<br>

##### #5 Extract **[IconV]**

to `MSYS\home\<Your Name>\libiconv\`.

<br>

##### #6 Start **MSYS**

and enter the following command:

```sh
cd libiconv && ./configure --disable-shared && make install
```

<br>

##### #7 Extract **[CMake]**

Place the `bin`, `doc`, `man` & `share` <br>folders into to your `MSYS` folder.

<br>

##### #8 Extract **[CEN64]**

Place the `Source` folders content in the <br>
`MSYS\home\<Your Name>\cen64\` directory.

<br>

##### #9 Start **MSYS**

and enter the following command:

```sh
cd cen64 && cmake-gui
```

<br>

##### #10 Configure CMake-GUI

<br>

Add the `MSYS/home/<Your Name>/cen64/` directory to

- `Browse Source...`
- `Browse Build...`

<br>

If not found add:

- `MSYS/mingw/x86_64-w64-mingw32/include/AL` as `OPENAL/OPENAL_INCLUDE_DIR`
- `MSYS/mingw/x86_64-w64-mingw32/lib/libOpenAL32.dll.a` as `OPENAL/OPENAL_LIBRARY`

<br>

Use **libiconv** as statically linked library, not **DLL**.

<br>

Set `MSYS/local/lib/libiconv.a` as `ICONV/ICONV_LIBRARIES`

<br>

Select `SSE4.1` or `AVX` as `Ungrouped Entries/CEN64_ARCH_SUPPORT`.

<br>

Make sure `VR4300_BUSY_WAIT_DETECTION` is ticked.

<br>

Click <kbd>Configure</kbd> and then <kbd>Generate</kbd> <br>
to create the **makefiles** for building.

<br>

You can now quit **CMake-GUI**.


<br>

##### #11 Make

Use make with:

```sh
make
```


<!----------------------------------------------------------------------------->

[MSYS]: https://sourceforge.net/projects/mingw/files/MSYS/Base/msys-core/msys-1.0.11/MSYS-1.0.11.exe/download

[MingW64]: https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z

[OpenAL]: https://openal-soft.org/openal-binaries/openal-soft-1.21.0-bin.zip

[IconV]: https://ftp.gnu.org/pub/gnu/libiconv/libiconv-1.16.tar.gz

[CMake]: https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4-win64-x64.zip

[CEN64]: https://github.com/n64dev/cen64
