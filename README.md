<p align="center">
  <img src="/assets/logo.png" />
</p>

# About:
This is my pet project. It's something I pick up whenever I get bored. To me,
what Nintendo and SGI did with this console is nothing short of amazing. The
ingenuity and design of the hardware was well-ahead of it's time, and it is
an absolute blast to reverse-engineer and study. I started this project in
order to learn more about what _really_ went on at the hardware level back in
the (good old) days.

That being said, I've also grown tired of people complaining that "cycle
accurate simulation on N-gen consoles is impossible." Not to be a bigot, but
no, it's not. Is it hard to write a simulator that is capable of running fast
enough to emulate a 93.75MHz processor on modern machines? Absolutely. Is it
impossible? No, certainly not. It just takes time. Programming is an art,
and like anything else, takes time if done well.

Getting back on track: this simulator attempts to be everything that every
other emulator hoped to be, but never quite attained due to... emulation. My
hopes are the properly simulating things at the cycle and pixel-accurate level
will preserve the original fidelity of the work that so many companies put
out.

Thank you to every single one of you developers for filling my childhood
with excellent memories. I'd also like to thank the community on all their
hard work and effort spend reverse-engineering this little gem. Without
further ado...

# Building:
The build system is, admittedly, a little hacked together. I'd like to get
something more maintainable and non-Unix friendly in the future (i.e., CMake).
Until then, I only support Makefiles.

For decent performance, your toolchain must absolutely be capable of some
kind of link-time optimization. If you use gcc/llvm, the default Makefiles
will already attempt to build with this feature. This is especially true when
linking all the static library plugins into the final executable; performance
will nosedive if the LTO isn't performed due to functions haved to be called
several million times per second.

The project has been broken into submodules in order to separate the commit
logs for each subsystem. Not only that, but it also is the most-amenable
way to handle a project of this (intended) scale. I can easily give members
commit access to individual parts of the project, or, I can also redirect one
of the default submodules to somebody else's repository altogether.


First, acquire all the sources:
  * Using git:
    * 'git submodule init'
    * 'git submodule update'

  * Manual method:
    * Extract a libaudio plugin to audio/
    * Extract a libbus plugin to bus/
    * Extract a libpif plugin to pif/
    * Extract a librdp plugin to rdp/
    * Extract a librdram plugin to rdram/
    * Extract a librom plugin to rom/
    * Extract a librsp plugin to rsp/
    * Extract a libvideo plugin to video/
    * Extract a libvr4300 plugin to vr4300/

Next, issue a build command:
  * Using GNU make: Issue a 'make' command.
  * Other build systems: Work in progress...

#  Running:
As of right now, you need ROM dumps from the retail console to run the
simulator. The PIF ("BIOS") image performs checksums and security checks on
the ROM image and initializes the environment. Please do not consult me on
regards to obtaining ROM images; this is illegal is most countries.

Once you obtain the images, simply execute:
cen64 <pifrom> <rom>

All ROM and binary images must be supplied in native (big-endian), or z64
format. Other formats will fail the CRC check and result in the virtual
console not booting.

