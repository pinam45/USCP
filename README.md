# Unicost Set Covering Problem (USCP)

This repository contain documents and code related to the research I made on the USCP during my internship at the [Institut de Recherche en Informatique, Mathématiques, Automatique et Signal (IRIMAS)](https://www.irimas.uha.fr/) laboratory of the [Université de Haute-Alsace (UHA)](https://www.uha.fr/).

The internship was from *2019-09-02* to *2020-02-07* and was my last year internship for the [Université de technologie de Belfort Montbéliard (UTBM)](https://www.uha.fr/) and the [Université du Québec à Chicoutimi (UQAC)](https://www.uqac.ca/).

The internship goal was to develop a memetic algorithm inspired from [Moalic2018] using the Row Weighting Local Search (RWLS) algorithm from [Gao2015] as a local search. The final version of the algorithm is named Memetic Algorithm for Set Covering (MASC) and was submitted to the [14th Learning and Intelligent OptimizatioN Conference](http://www.caopt.com/LION14/) (LION14).

The LION14 submission is "**A memetic approach on the Unicost Set Cover Problem**" by

| Name                | Email                      | Institute |
|:--------------------|:---------------------------|:---------:|
| Maxime Pinard       | maxime.pin@live.fr         |    1,2    |
| Laurent Moalic      | laurent.moalic@uha.fr      |    1,2    |
| Mathieu Brévilliers | mathieu.brevilliers@uha.fr |    1,2    |
| Julien Lepagnot     | julien.lepagnot@uha.fr     |    1,2    |
| Lhassane Idoumghar  | lhassane.idoumghar@uha.fr  |    1,2    |

| Ref. | Institute                                                            |
|:----:|:---------------------------------------------------------------------|
|  1   | Université de Haute-Alsace, IRIMAS UR 7499, F-68100 Mulhouse, France |
|  2   | Université de Strasbourg, France                                     |

## documents

This folder contains documents written in LaTeX. To compile the documents to PDF files, you will need a LaTeX distribution such as [TeX Live](http://tug.org/texlive) that include [LuaTeX](http://www.luatex.org/) installed on your system.

For each document, a ``.latexmkrc`` is provided so the document can be compiled by running *latexmk* in the document directory:
```
$ latexmk -silent
```

### instances

This document contain summary tables of instances used to benchmark the implemented methods. The instances described and several more information can be found in [USCP_instances](https://github.com/pinam45/USCP_instances).

## code

This directory contain the C++ code of the project, the project was implemented in C++17 and uses CMake to configure the build, it is split in 3 targets, a common library and two programs: solver and printer.

### common

This library contain the basic structures used to represent the USCP, the instances reader and writer functions, all serializable classes, some utility functions and logging related stuff.

It also contain the database of "known" instances from [USCP_instances](https://github.com/pinam45/USCP_instances):
```
4.1,4.2,4.3,4.4,4.5,4.6,4.7,4.8,4.9,4.10,5.1,5.2,5.3,5.4,5.5,5.6,5.7,5.8,5.9,5.10,6.1,6.2,6.3,6.4,6.5,A.1,A.2,A.3,A.4,A.5,B.1,B.2,B.3,B.4,B.5,C.1,C.2,C.3,C.4,C.5,D.1,D.2,D.3,D.4,D.5,E.1,E.2,E.3,E.4,E.5,NRE.1,NRE.2,NRE.3,NRE.4,NRE.5,NRF.1,NRF.2,NRF.3,NRF.4,NRF.5,NRG.1,NRG.2,NRG.3,NRG.4,NRG.5,NRH.1,NRH.2,NRH.3,NRH.4,NRH.5,CLR10,CLR11,CLR12,CLR13,CYC6,CYC7,CYC8,CYC9,CYC10,CYC11,RAIL507,RAIL516,RAIL582,RAIL2536,RAIL2586,RAIL4284,RAIL4872,STS9,STS15,STS27,STS45,STS81,STS135,STS243,STS405,STS729,STS1215,STS2187
```

### solver

This program solve USCP instances with different algorithms and parameters and generate json reports with the resulting solution and all the resolution information (solving time, steps made,...).

This program implements 3 algorithms:
 - A deterministic greedy algorithm
 - Row Weighting Local Search (RWLS)
 - Memetic Algorithm for Set Covering (MASC)

It is a command line program, possible options are given by ``solver --help``:
```
      --help                    Print help
      --version                 Print version
  -i, --instances NAME          Instances to process
      --instance_type orlibrary|orlibrary_rail|sts|gvcp
                                Type of the instance to process
      --instance_path PATH      Path of the instance to process
      --instance_name NAME      Name of the instance to process
  -o, --output_prefix PREFIX    Output file prefix (default: solver_out_)
  -r, --repetitions N           Repetitions number (default: 1)
      --greedy                  Solve with greedy algorithm (no repetition as
                                it is determinist)
      --rwls                    Improve with RWLS algorithm (start with a
                                greedy)
      --rwls_steps N            RWLS steps limit (default:
                                18446744073709551615)
      --rwls_time N             RWLS time (seconds) limit (default:
                                18446744073709551615)
      --memetic                 Solve with memetic algorithm
      --memetic_cumulative_rwls_steps N
                                Memetic cumulative RWLS steps limit (default:
                                18446744073709551615)
      --memetic_cumulative_rwls_time N
                                Memetic cumulative RWLS time (seconds) limit
                                (default: 18446744073709551615)
      --memetic_time N          Memetic time limit (default:
                                18446744073709551615)
      --memetic_crossover OPERATOR
                                Memetic crossover operator (default: default)
      --memetic_wcrossover OPERATOR
                                Memetic RWLS weights crossover operator
                                (default: default)
```

The known instances are those from the common library, they can be specified using ``--instances=<comma separated list of instances>``, for an unknown instance, use ``--instance_type=<orlibrary|orlibrary_rail|sts|gvcp> --instance_path=<path> --instance_name=<name>``.

For the Memetic algorithm, possible crossovers are:
 - **identity**: return the first parent for the first child, return the second parent for the second child
 - **merge**: return a solution with subsets from both parents
 - **greedy_merge**: return a solution generated by taking greedily and alternatively subsets from the first and the second parent
 - **subproblem_random**: return a solution generated by launching a random resolution on the subproblem generated with subsets from both parents
 - **extended_subproblem_random**: return a solution generated by launching a random resolution on the subproblem generated with subsets from both parents and a random solution
 - **subproblem_greedy**: return a solution generated by launching a greedy resolution on the subproblem generated with subsets from both parents
 - **extended_subproblem_greedy**: return a solution generated by launching a greedy resolution on the subproblem generated with subsets from both parents and a random solution
 - **subproblem_rwls**: return a solution generated by launching RWLS on the subproblem generated with subsets from both parents
 - **extended_subproblem_rwls**: return a solution generated by launching RWLS on the subproblem generated with subsets from both parents and a random solution

and possible wcrossover are:
 - **reset**: return a vector with all weights equal to 1
 - **keep**: return a vector with the weights of the first parent for the first child and the weights of the second parent for the second child
 - **average**: return a vector in which each weight is the average of the corresponding weights in the parents
 - **mix_random**: return a vector in which each weight taken randomly from one of the parents
 - **add**: return a vector in which each weight is the sum of the corresponding weights in the parents
 - **difference**: return a vector in which each weight is the difference of the corresponding weights in the parents
 - **max**: return a vector in which each weight is the maximum of the corresponding weights in the parents
 - **min**: return a vector in which each weight is the minimum of the corresponding weights in the parents
 - **minmax**: return a vector with each weight being the minimum of the corresponding weights in the parents for the first child, and each weight being the maximum of the corresponding weights in the parents for the second child
 - **shuffle**: return a vector with the weights of the first parent shuffled for the first child and the weights of the second parent shuffled for the second child

Usage examples:
 - Solve CYC10 and CYC11 instances with RWLS and a limit of 5000 steps:
```
./solver --instances=CYC10,CYC11 --rwls --rwls_steps=5000
```
 - Solve R42, an unknown RAIL instance in ``./rail_42.txt`` using the same format as in OR-Library, with the Memetic algorithm, the subproblem_rwls crossover, the max wcrossover and a limit of 360 seconds:
```
./solver --instance_type=orlibrary_rail --instance_path=./rail_42.txt --instance_name=R42 --memetic --memetic_crossover=subproblem_rwls --memetic_wcrossover=max --memetic_time=360
```

For more example, see the benchmarks results and launching scripts in [USCP_results](https://github.com/pinam45/USCP_results).

The program contain a lot of debug asserts and the CMake config will link the address, leak, undefined behaviour and memory sanitizers in debug. When not debugging or testing the solver, please compile the in release to be efficient.

### printer

This program take as input the reports generated by the solver and process them to generate a LaTeX document containing instances information tables, a results table and optionally a RWLS statistics table, RWLS weights plots and memetic comparison tables.

It is a command line program, possible options are given by ``printer --help``:
```
      --help                  Print help
      --version               Print version
  -i, --input FOLDER/FILE     Input folders/files to process
  -o, --output_prefix PREFIX  Output file prefix (default: printer_out_)
  -v, --validation REGEX      Input files name validation regex (default:
                              .*\.json)
      --rwls_stats            Generate RWLS stats table
      --rwls_weights          Generate RWLS weights plots
      --memetic_comparisons   Generate memetic comparison tables
```

Usage example:
 - Generate result document with RWLS stats for solver reports stored in ./solver_out:
```
./printer --input=./solver_out --rwls_stats
```

For more example, see the arguments used for the benchmarks in [USCP_results](https://github.com/pinam45/USCP_results).

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

Project CMake targets:
 - **common**: build the common library
 - **solver**: build the solver
 - **printer**: build the printer
 - **format-common**: format the  common library code
 - **format-solver**: format the solver code
 - **format-printer**: format the printer code
 - **git_info** / **update_git_info**: generate the git_info.hpp file with the git current commit information

### Run

The programs must be run in the folder containing the resources folder, otherwise instances and documents templates can't be loaded.

## References

```BibTeX
@Article{Gao2015,
  author       = {Chao Gao and Xin Yao and Thomas Weise and Jinlong Li},
  title        = {An efficient local search heuristic with row weighting for the unicost set covering problem},
  journaltitle = {European Journal of Operational Research},
  date         = {2015},
  volume       = {246},
  number       = {3},
  pages        = {750--761},
  issn         = {0377-2217},
  doi          = {10.1016/j.ejor.2015.05.038},
  groups       = {Local search, Reviewed},
  keywords     = {Combinatorial optimization, Unicost set covering problem, Row weighting local search},
}
@Article{Moalic2018,
  author       = {Moalic, Laurent and Gondran, Alexandre},
  title        = {Variations on memetic algorithms for graph coloring problems},
  journaltitle = {Journal of Heuristics},
  date         = {2018-02},
  volume       = {24},
  number       = {1},
  pages        = {1--24},
  issn         = {1572-9397},
  doi          = {10.1007/s10732-017-9354-9},
  keywords     = {Combinatorial optimization, Metaheuristics, Coloring, Graph, Evolutionary},
  publisher    = {Springer US},
}
```

## License

<img align="right" src="http://opensource.org/trademarks/opensource/OSI-Approved-License-100x137.png">

USCP is licensed under the [MIT License](http://opensource.org/licenses/MIT):

Copyright &copy; 2019 [Maxime Pinard](https://github.com/pinam45)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
