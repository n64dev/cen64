# Usage

* How do I run cen64?<br />
You will need a valid pifdata.bin file (NTSC or PAL), & a ROM in .z64 format (in big-endian format).<br />

To run cen64 without multithreading (slower):<br />
cen64 pifdata.bin ROM.z64<br />
<br />
To run cen64 with multithreading (faster):<br />
cen64 -multithread pifdata.bin ROM.z64<br />

* How do I run 64DD games?<br />
You will need a valid 64ddipl.bin file (NTSC JPN or USA), & a 64DD disk image file in .ndd format.<br />

cen64 -ddipl 64ddipl.bin -ddrom DISK.ndd pifdata.bin<br />

* How do I setup save files for games?<br />
N64 has various types of save formats used in games...<br />
You will need to specify the save type yourself, cen64 will create the file if it does not exist.<br />

EEP4K:<br />
cen64 -eep4k eep4k.bin pifdata.bin ROM.z64<br />
<br />
EEP16K:<br />
cen64 -eep16k eep16k.bin pifdata.bin ROM.z64<br />
<br />
FLASH:<br />
cen64 -flash flash.bin pifdata.bin ROM.z64<br />
<br />
SRAM:<br />
cen64 -sram sram.bin pifdata.bin ROM.z64<br />

* The game runs, but I get strange errors, like multiple ocarinas in Majora's Mask.<br />

You can fix the issue by using the -flash option on the command line:<br />
cen64 -flash flash.bin pifdata.bin majora.z64
