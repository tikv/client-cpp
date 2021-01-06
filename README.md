# TiKV Client for C++

This library is the TiKV client for C++; it only supports synchronous API so far.

It's built on top of 
[TiKV Client in Rust](https://github.com/tikv/client-rust) via [cxx](https://github.com/dtolnay/cxx). 

This client is still in the stage of prove-of-concept and under heavy development.

# compilation process

## all

```bash
make all
```

## build static lib

```bash
make build-lib
```

## output target

- libtikv_client.a : static lib
- tikv-test: execute file for test
