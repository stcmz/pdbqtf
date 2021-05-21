pdbqtf
=====
![build workflow](https://github.com/stcmz/pdbqtf/actions/workflows/build.yml/badge.svg)
![release workflow](https://github.com/stcmz/pdbqtf/actions/workflows/release.yml/badge.svg)

pdbqtf is a command line tool for fixing problematic chemical elements in PDBQT files.


Features
--------

* fix problematic question mark (?) elements in PDBQT files
* batch fixing
* in-place fixing or save as new file

pdbqtf accepts the following ways of input:
* standard input: `pdbqtf`
* file input: `pdbqtf file.pdbqt`
* input redirect: `pdbqtf -L <(cat file.pdbqt)`
* pipe input: `cat file.pdb | pdbqtf`


Supported operating systems and compilers
-----------------------------------------

All systems with compilers in conformance with the C++17 standard, e.g.
* Linux x86_64 and g++ 8.3.1 or higher
* Mac OS X x86_64 and clang 7 or higher
* Windows x86_64 and msvc 19.14 or higher


Compilation from source code
----------------------------

### Get Boost

pdbqtf depends on the `Program Options` component in [Boost C++ Libraries]. Boost 1.75.0 was tested. There are several ways to get Boost.

#### With `vcpkg` on Windows, macOS or Linux:
```
# Note: this will download and build from source
vcpkg install boost-program-options
```

#### With `nuget` on Windows:
```
nuget install boost_program_options-vc142
```

#### With `apt` on Ubuntu/Debian:
```
sudo apt install libboost-program-options-dev
```

#### With `Homebrew` on macOS:
```
brew install boost
```

#### With `PowerShell` on Windows:
```
$Url = "https://sourceforge.net/projects/boost/files/boost-binaries/1.75.0/boost_1_75_0-msvc-14.2-64.exe"
(New-Object System.Net.WebClient).DownloadFile($Url, "$env:TEMP\boost.exe")
Start-Process -Wait -FilePath "$env:TEMP\boost.exe" "/SILENT","/SP-","/SUPPRESSMSGBOXES"
```

#### Build from Source on Linux or Windows:

Download [Boost 1.75] and unpack the archive to `boost_1_75_0/include`.

Build on Linux run:
```
cd boost_1_75_0/include
./bootstrap.sh
./b2 --build-dir=../build/linux_x64 --stagedir=../ -j 8 link=static address-model=64
```

Or, on Windows run:
```
cd boost_1_75_0\include
bootstrap.bat
b2 --build-dir=../build/win_x64 --stagedir=../ -j 8 link=static address-model=64
```

Then add the path of the `boost_1_75_0` directory the to the BOOST_ROOT environment variable.

### Build with CMake

This project uses cross-platform build system CMake to build from source. It detects your environment and decides the most appropriate compiler toolset. The minimum version of CMake required is `3.20`. To build, simply run
```
cmake -B build
cmake --build build --config Release
```

The generated objects and executable will be placed in the `build` folder.

Optionally, on Linux or macOS one may install the output binary to the system (usually `/usr/local/bin`) by running
```
sudo cmake --install build
```

On Windows, the script should be run without sudo but under Administrator. The executable will be copied to an individual directory under `Program Files`.


### Build with Visual Studio

Visual Studio 2019 solution and project files are provided. To compile, simply run
```
msbuild /t:Build /p:Configuration=Release
```

Or one may open `pdbqtf.sln` in Visual Studio 2019 and do a full rebuild.

The generated objects will be placed in the `obj` folder, and the generated executable will be placed in the `bin` folder.


Usage
-----

First add pdbqtf to the PATH environment variable.

To display a full list of available options, simply run the program with the `--help` argument
```
pdbqtf --help
```

See the [Features section](#features) above for usages in different input ways.


Author
--------------

[Maozi Chen]


[Boost C++ Libraries]: https://www.boost.org
[Maozi Chen]: https://www.linkedin.com/in/maozichen/
[Boost 1.75]: https://www.boost.org/users/history/version_1_75_0.html
