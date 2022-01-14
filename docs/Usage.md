
# Usage

### How do I run CEN64 ?

You will need a valid `pifdata.bin` file as well as a **ROM**.

##### .bin

Can be `NTSC` or `PAL`.

##### ROM

Has to be of the `.64` / **Big Endian** format.

<br>

##### Multithreading

To make **CEN64** faster, you can use mutlithreading, for <br>
this simply add the `-multithread` command line option.

```sh
cen64 -multithread pifdata.bin ROM.z64
```

<br>

---

### How do I run 64DD Games ?

You will need a valid `64ddipl.bin` file and a `64DD` disk image.

##### .bin

Should be in the `NTSC` format, <br>
both **JPN** / **USA** version work.

##### Disk Image

Has to be in the `.ndd` format.

<br>

```sh
cen64 -ddipl 64ddipl.bin -ddrom DISK.ndd pifdata.bin
```

<br>

---

### How do I set up game save files ?

There are various formats to choose from, <br>
once specified, **CEN64** will create them.

##### EEP4K

```sh
cen64 -eep4k eep4k.bin pifdata.bin ROM.z64
```

##### EEP16K

```sh
cen64 -eep16k eep16k.bin pifdata.bin ROM.z64
```

##### FLASH

```sh
cen64 -flash flash.bin pifdata.bin ROM.z64
```

##### SRAM

```sh
cen64 -sram sram.bin pifdata.bin ROM.z64
```

<br>

---

### Strange Errors

*The game runs but I get strange errors,* <br>
*like multiple ocarinas in Majora's Mask.*

➜ Use the `-flash` command line option.

```sh
cen64 -flash flash.bin pifdata.bin majora.z64
```
