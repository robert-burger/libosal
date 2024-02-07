# libosal

## Usage 

See INTRODUCTION.md or gh-pages for reference.

## Build from source

`libosal` uses autotools as build system. To build from source execute something like:

```
git clone https://github.com/robert-wm/libosal.git
cd libosal
autoreconf -is
./configure
make
sudo make install
```

This will build and install a static as well as a dynamic library. For use in other project you can you the generated pkg-config file to retreave cflags and linker flags.
