# sv-toolchain

Tools for system-verilog language

## features

- Lexer : raw system-verilog code (per source-file) to token-stream

## resource

Head over to [Wiki](https://github.com/circuit555/sv-toolchain/wiki)
section for helpful reference material.

## build

### docker based workflow

#### build and launch docker

```shell
# build docker image if not already on your system
$ docker build -f <this-repo>/dockers/x64-linux-dev/sv-toolchain.dockerfile \
               -t sv-toolchain-dev .
```

```
# launch container
$ docker run -it --rm -e LD_LIBRARY_PATH="/modules/lib;/modules/lib/uzleo" \
<your-flag-soup> sv-toolchain-dev
```

#### inside docker

```shell

# clone this repo and navigate into it
$ git clone git@github.com:circuit555/sv-toolchain --recursive
$ cd sv-toolchain

# generated build files using modi tool
$ modi

# generate compile_commands.json at repo root (for clangd/clang-tidy)
$ python3 scripts/generate_compile_commands.py

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

If `build.json` changes (for example, new source files or modules), run:

```shell
$ modi
$ python3 scripts/generate_compile_commands.py
```
