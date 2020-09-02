# YACLC - Yet another COOL language compiler

## Introduction

This repository contains an implementation of the COOL programming language. Differently from most classroom-based implementations, the entire project was built from scratch for personal fun.

The compiler takes a source file as its lone argument and translates the program into MIPS assembly. The compiler output is returned to the standard output. The compiler executable is named, not surprisingly, `cool`. An example usage is shown below:

    ./cool path_to_source_file

The compiler itself is structured into three main components, organized into separate libraries:

- a frontend, powered by Flex and Bison;
- a semantic analyzer;
- a MIPS code generator.

The semantic analyzer is responsible for type checking as well as other mundane tasks, such as ensuring that attributes are defined only once and that no cyclic class dependency exists. The code generator is based on a generic register machine which maintains the following stack invariation: for each expression, the generated code is guaranteed not to change the stack pointer value.

## Installation

The project uses `CMake` as its built tool and requires `C++14`. A straightforward way of building the project is to create a `build` directory, configure the build files and invoke `make` as follows:

    mkdir build
    cd ..
    cmake ..
    make

If you are interested in more sophisticated build options, please visit https://gitlab.kitware.com/cmake/cmake.

## Future versions

In the immediate future, we plan to include a `Dockerfile` in this repository to allow users to build the compiler and work with it from within a container. We also plan to include the MIPS simulator `spim` inside the image. Long term, we want to include a code optimization pass to the compiler so to generate more performant assembly code.
