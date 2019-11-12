# USCP

This repository contain documents and code related to the research I made on the **Unicost Set Covering Problem** during my internship at the [Institut de Recherche en Informatique, Mathématiques, Automatique et Signal (IRIMAS)](https://www.irimas.uha.fr/) laboratory of the [Université de Haute-Alsace (UHA)](https://www.uha.fr/).

The internship was from *2019-09-02* to *2020-02-07* and was my last year internship for the [Université de technologie de Belfort Montbéliard (UTBM)](https://www.uha.fr/) and the [Université du Québec à Chicoutimi (UQAC)](https://www.uqac.ca/).

## documents

This folder contains the documents written in LaTeX. To compile the documents to PDF files, you will need a LaTeX distribution such as [TeX Live](http://tug.org/texlive) that include [LuaTeX](http://www.luatex.org/) installed on your system.
 
For each document, a ``.latexmkrc`` is provided so the document can be compiled by running *latexmk* in the document directory:
```
$ latexmk -silent
```

### instances

This document contain a summary tables of instances used to benchmark the implemented methods. The instances described and several more information can be found in [USCP_instances](https://github.com/pinam45/USCP_instances).

## code

This directory contain the C++ code of the project split in 3 targets, a common library (with serialization classes, logging utilities, ...) and two programs: solver and printer.

### Dependencies

Libraries used:
- [CMutils](https://github.com/pinam45/CMutils): CMake utilities for C/C++ projects
- [dynamic_bitset](https://github.com/pinam45/dynamic_bitset): C++17 header-only dynamic bitset 
- [spglog](https://github.com/gabime/spdlog): Fast C++ logging library
- [inja](https://github.com/pantor/inja): Template Engine for Modern C++
- [json](https://github.com/nlohmann/json): JSON for Modern C++
- [cxxopts](https://github.com/jarro2783/cxxopts): Lightweight C++ command line option parser

All libraries are included as git submodule and built with the project, they don't need to be installed.

### Build

Don't forget to pull the submodules with the repository using ``--recursive``:
```
git clone --recursive https://github.com/pinam45/USCP.git
```

or if you have already cloned the repository:
```
git submodule update --init
```

To build the project you will need a **C++17 compliant compiler** and **CMake 3.10 or later**. The project uses CMake to build, see [Running CMake](https://cmake.org/runningcmake/) for more information.

On linux, a common CMake use is:
```
$ mkdir cmake-build
$ cd cmake-build
$ cmake ..
$ make
```

On Windows, there is batch files available to configure a Visual Studio project in the [ide](code/ide) folder.

CMake options:

| Option                         | Default value |                Description                |
|:-------------------------------|:-------------:|:-----------------------------------------:|
| USCP_RWLS_LOW_MEMORY_FOOTPRINT |      OFF      | Enable RWLS low memory footprint (slower) |

### Run

The programs must be run in the folder containing the resources folder, otherwise instances and document templates can't be loaded.

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

USCP is licensed under the [MIT License](http://opensource.org/licenses/MIT):

Copyright &copy; 2019 [Maxime Pinard](https://github.com/pinam45)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
