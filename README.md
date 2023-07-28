# TiKV Client for C++

TiKV client for C++. So far, it only supports synchronous API.

It's built on top of
[TiKV Client in Rust](https://github.com/tikv/client-rust) via [cxx](https://github.com/dtolnay/cxx).

This client is still in the stage of prove-of-concept and under heavy development.

## Prepare 

```bash
# install rust environment
curl https://sh.rustup.rs -sSf | sh
```

## Build

```bash
## compile in build directory
cmake -S . -B build && cmake --build build
## install to /usr/local
sudo cmake --install build
```


## Run example

```bash
# run with tikv-server
tiup playground nightly

cd examples && cmake -S . -B build && cmake --build build
# run raw example
./build/raw
```
