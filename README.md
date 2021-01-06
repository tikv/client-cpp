# TiKV Client for C++

TiKV client for C++. So far, it only supports synchronous API.

It's built on top of 
[TiKV Client in Rust](https://github.com/tikv/client-rust) via [cxx](https://github.com/dtolnay/cxx). 

This client is still in the stage of prove-of-concept and under heavy development.

## Build

```bash
cargo install cxxbridge-cmd --force --version 1.0.18
make
```

Then the library will be in `target/debug/libtikv_client.a`.

Otherwise, you can build release version by the following. The library will be in
`target/release/libtikv_client.a`.

```bash
make release
```

## Run example

```bash
tiup playground
make run-example
```
