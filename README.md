# sv-toolchain
tools for system-verilog language

## features
- Lexer : raw system-verilog code (per source-file) to token-stream

## resource

head over to [Wiki](https://github.com/circuit555/sv-toolchain/wiki) section for helpful reference material.

## build

### docker based workflow

#### first run

```shell
# build docker image if not already on your system
$ docker build -f dockers/sv-toolchain.dockerfile -t sv-toolchain-dev .

# launch container
$ docker run -it --rm -e LD_LIBRARY_PATH="/modules/lib;/modules/lib/uzleo" <your-flag-soup> sv-toolchain-dev

# inside docker shell ...

# clone this repo and navigate into it
$ git clone git@github.com:circuit555/sv-toolchain --recurse
$ cd sv-toolchain

# generated build files using modi tool
$ modi

# build project
$ ninja -f build/build.ninja

# run tool
$ build/svt
```

#### further runs

```shell
# source code modified ...
$ ninja -f build/build.ninja
$ build/svt
```
