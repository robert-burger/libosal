# libosal

[![Build and Publish Debian Package](https://github.com/robert-burger/libosal/actions/workflows/build-deb.yaml/badge.svg)](https://github.com/robert-burger/libosal/actions/workflows/build-deb.yaml)
[![License: LGPL-V3](https://img.shields.io/badge/license-LGPL--V3-green.svg)](LICENSE)
[![Linux](https://img.shields.io/badge/Linux-FCC624?logo=linux&logoColor=black)](#)
[![Debian](https://img.shields.io/badge/Debian-A81D33?logo=debian&logoColor=fff)](#)
[![Ubuntu](https://img.shields.io/badge/Ubuntu-E95420?logo=ubuntu&logoColor=white)](#)
[![PikeOS](https://img.shields.io/badge/PikeOS-purple.svg)](LICENSE)
[![VxWorks](https://img.shields.io/badge/VxWorks-yellow.svg)](LICENSE)
[![Windows](https://custom-icon-badges.demolab.com/badge/Windows-0078D6?logo=windows11&logoColor=white)](#)

[![Packaging status](https://repology.org/badge/vertical-allrepos/libosal.svg)](https://repology.org/project/libosal/versions)

## 🍀 Overview

**libosal** is a high-performance, modular Operating System Abstraction Layer (OSAL) designed to facilitate cross-platform development by abstracting platform-specific details of operating systems and hardware environments. Developed by the Institute of Robotics and Mechatronics at the German Aerospace Center (DLR), libosal enables seamless portability and maintainability of embedded and real-time software components across diverse operating systems including WIN32, MINGW32, PIKEOS, POSIX-compliant systems, and VXWORKS.

By encapsulating OS-dependent functionality behind a uniform API, libosal empowers developers to write hardware-agnostic, reusable, and robust code that is decoupled from platform-specific implementation details, thereby accelerating development cycles and improving software reliability in complex mechatronic and aerospace applications.

---

## ✨ Key Features

- **Cross-Platform Compatibility**  
  Supports a wide range of real-time and general-purpose operating systems such as Windows (WIN32), POSIX-compliant UNIX variants (Linux, macOS), PikeOS microkernel OS, and VxWorks real-time OS. This flexibility allows integration into heterogeneous system landscapes with minimal code modification.

- **Modular and Extensible Architecture**  
  Designed with modularity at its core, libosal can be extended or customized to support new platforms or specialized functionalities while preserving API stability. This modular approach fosters maintainability and ease of integration within complex software stacks.

- **Rich API for System Services**  
  Provides abstractions for critical system services including threading, synchronization primitives, timers, memory management, and device I/O. This facilitates uniform access to essential OS features irrespective of the underlying platform.

- **Robust Build System Support**  
  Supports both Autotools and CMake build environments, enabling smooth integration into diverse CI/CD pipelines and build infrastructures. This ensures consistent and reproducible builds across different development environments.

- **Comprehensive Test Suite**  
  Includes platform-specific and platform-independent test suites, enabling rigorous validation of functionality and performance. Testing infrastructure ensures reliability and correctness across supported platforms.

---

## 📄 Licensing and Contribution

libosal is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0), enabling both open-source and proprietary usage with compliance.

Contributions are welcome via GitHub pull requests. Please adhere to the project’s Code of Conduct and Contribution Guidelines.

---

## 🤝 Contributing, Contact and Support

Developed and maintained by the [Institute of Robotics and Mechatronics of the German Aerospace Center (DLR)](https://www.dlr.de/rm). For inquiries, bug reports, or feature requests, please open an issue on the GitHub repository or contact the maintainers directly.

Contributions, pull requests, and issue reports are always welcome. Please:

- Keep builds warning-free
- Stick to robotkernel style rules
- Add tests for new features or regressions

---

## Usage 

See [introduction](INTRODUCTION.md) or [gh-pages](https://robert-burger.github.io/libosal) for reference.

---

## Install

There's a debian repository hosted by myself where you can automatically install libosal via apt (Ubuntu 22.04, 24.04 and debian bullseye, bookworm). To setup the repository enter:

```bash
curl -fsSL https://deb.burger-system.de/setup.sh | bash > /dev/null
```

Afterwards you should be able to install libosal simply with:

```bash
sudo apt update
sudo apt install libosal
```

---

## 🐊 Build from source

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

---

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

### 🧩 Configuration parameters

| Parameter                            | Default | Description                                                               |
|--------------------------------------|---------|---------------------------------------------------------------------------|
| BUILD_FOR_PLATFORM                   |         | Select manually your platform (WIN32, MINGW32, PIKEOS, POSIX, or VXWORKS) |
| BUILD_SHARED_LIBS                    |   OFF   | Flag to build shared libraries instead of static ones.                    |
| BUILD_WITH_POSITION_INDEPENDENT_CODE |   OFF   | Flag to build with -fpic option´. Required for shared libs                |

---

## 🧪 Tests

On POSIX, run

```
make check
```

See tests/posix/README.md for details.

For Cissy/Conan, you can also set the option "coverage"
to produce coverage analysis in HTML format for the 
POSIX implementation.


