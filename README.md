# TiKV Client for C++

TiKV client for C++. So far, it only supports synchronous API.

It's built on top of
[TiKV Client in Rust](https://github.com/tikv/client-rust) via [cxx](https://github.com/dtolnay/cxx).

This client is still in the stage of prove-of-concept and under heavy development.

## Build

### Local Build
```bash
# cxxbridge-cmd 1.0.18 requires rustc 1.48+ and c++17 or newer
cargo install cxxbridge-cmd --force --version 1.0.18
make
```

Then the library will be in `target/debug/libtikv_client.a`.

Otherwise, you can build release version by the following. The library will be in
`target/release/libtikv_client.a`.

```bash
make release
```
### Docker build
**Way 1: Use Visual Studio Code Dev Container**
you can use the dev container to build the project, just open the project in VSCode and press `F1` and select `Dev Containers: Reopen in Container` to open the project in dev container.

**Way 2: Build Docker Locally Then Compile**
```bash
docker build -t tikv/client-cpp:latest \ -f .devcontainer/Dockerfile .
docker run -v $(pwd):/client-cpp \
        tikv/client-cpp:latest \
        /bin/bash -c "make release"
```

**Way3: Use Image to Compile(Image is NOT Official)**
```bash
docker run -v $(pwd):/client-cpp \
        registry.cn-hangzhou.aliyuncs.com/smityz/client-cpp:latest \
        /bin/bash -c "make release"
```


## Run example

```bash
tiup playground nightly
# run rawkv example
make run-raw-example
# run txnkv example
make run-txn-example        
```
