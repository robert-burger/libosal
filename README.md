# libosal

libosal is an operating system abstraction layer Library. It's purpose is to write os-independent code for easy portability between different systems and architectures. 

This software was developed and made available to the public by the [Institute of Robotics and Mechatronics of the German Aerospace Center (DLR)](https://www.dlr.de/rm).

## Usage 

See [introduction](INTRODUCTION.md) or gh-pages for reference.

## Build from source

`libosal` uses autotools as build system. To build from source execute something like:

```
git clone https://github.com/robert-burger/libosal.git
cd libosal
autoreconf -is
./configure
make
sudo make install
```


This will build and install a static as well as a dynamic library. For use in other project you can you the generated pkg-config file to retreave cflags and linker flags.

## Build with CMake

```bash
mkdir build
cd build
# You can choose MINGW32, POSIX, WIN32, PIKEOS, or VXWORKS. No cross compile currently supported with CMake
cmake -DBUILD_FOR_PLATFORM="POSIX" -DBUILD_SHARED_LIBS=ON ..
cmake --build .
# You can define a specific install path with e.g. cmake --install .  --prefix test
cmake --install . 
```

### Configuration parameters

| Parameter                            | Default | Description                                                               |
|--------------------------------------|---------|---------------------------------------------------------------------------|
| BUILD_FOR_PLATFORM                   |         | Select manually your platform (WIN32, MINGW32, PIKEOS, POSIX, or VXWORKS) |
| BUILD_SHARED_LIBS                    |   OFF   | Flag to build shared libraries instead of static ones.                    |
| BUILD_WITH_POSITION_INDEPENDENT_CODE |   OFF   | Flag to build with -fpic option´. Required for shared libs                |


## Tests

On POSIX, run

```
make check
```

See tests/posix/README.md for details.

For Cissy/Conan, you can also set the option "coverage"
to produce coverage analysis in HTML format for the 
POSIX implementation.


