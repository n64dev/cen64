
# Debugging with GDB

This is a short tutorial on how to use the cen64 debugger

## Including Debugging Symbols

When compiling your C source files you need to include debug symobls. Using using GCC, you do this with the -g flag

```bash
mips64-elf-cc -c -g -o src/boot.o src/boot.c
```

This will include debugging symbols into `src/boot.o`. You can verify debugging symbols have been included in the .o by attempting to load it into gdb using `gdb -q <path_to_.o>`

This what it looks like when there are symbols
```bash
gdb -q src/boot.o
Reading symbols from src/boot.o...
(gdb) 
```

This is what it looks like when there are no symbols
```bash
gdb -q src/boot.o
Reading symbols from src/boot.o...
(No debugging symbols found in src/boot.o)
(gdb) 
```


Next you need to make sure the symbols are preserved in the linking process. This is the default behavoir of `ln` but if you find that the `.o` files that result from the linking process don't contain debugging symbols then they may be getting stripped out. This could either be from the `-S` flag used as in input to the `ld` program in they are getting removed as part of the linker script. If you are using spicy, it is configured to remove symbols by default. My fork of [spicy](https://github.com/lambertjamesd/spicy) it will not strip debug symbols.

The `.o` file generated from spicy or, if you aren't using spicy, the `.o` file used in `objcopy` to generate the rom file is the one you should load into gdb to get the symbols for the rom. `objcopy` should strip debugging symbols for you so there is no problem keeping them in during the final linker step.

## Starting cen64 with the Debugger

To start cen64 with the debugger, simply run it with the `-debug` flag.

```
cen64 -debug localhost:2345 pifdata.bin game.n64
```

This will cause cen64 to pause and listen on port `2345` for an incoming connection from gdb. It will not run until gdb has connected.

## Connecting with GDB

After you run cen64, you can then run and connect gdb to it. I found I had to use `gdb-multiarch` to get debugging with a MIPS processor to work. Before connecting to cen64 you need to start up gdb with the correct symbols

```
gdb-multiarch -q game.out
```

If everything worked you should see this

```
gdb-multiarch -q game.out
Reading symbols from game.out...
(gdb) 
```

You can then type the following command into gdb

```
target remote localhost:2354
```

If everything is working it should look like this.

```
(gdb) target remote localhost:8080
Remote debugging using localhost:8080
0x80000000 in ?? ()
(gdb) 
```

At this point gdb is working and ready to go. The debugger is paused in the boot code. You can set breakpoints using the `break` command.

This this example, main is the name of the entrypoint into the code.
```
break main
``` 

You can begin running your rom by using the command `c`.

```
c
```

This is not a tutorial on how to use gdb. If you need further help on how to debug you will have to look elsewhere. The recommened way to use this however is to find a gdb plugin for an IDE to handle the low level commands for you.

