pes_parent_dir:=$(shell pwd)/$(lastword $(MAKEFILE_LIST))
cur_makefile_path := $(shell dirname $(pes_parent_dir))

all: build build-example target/tikv-example

build: pre-build target/debug/libtikv_client.a

release: pre-build target/release/libtikv_client.a

pre-build: target/tikv_client_glue.cc include/tikv_client_glue.h
	mkdir target

clean:
	cargo clean

run-example: target/tikv-example
	RUST_LOG=debug $(cur_makefile_path)/target/tikv-example

target/tikv-example: target/debug/libtikv_client.a example/main.cpp
	c++ $(cur_makefile_path)/example/main.cpp -o $(cur_makefile_path)/target/tikv-example -std=c++17 -g -I$(cur_makefile_path)/include -L$(cur_makefile_path)/target/debug -ltikv_client -lpthread -ldl -lssl -lcrypto


target/tikv_client_glue.cc: src/lib.rs
	cxxbridge $(cur_makefile_path)/src/lib.rs > $(cur_makefile_path)/target/tikv_client_glue.cc

include/tikv_client_glue.h: src/lib.rs
	cxxbridge $(cur_makefile_path)/src/lib.rs --header > $(cur_makefile_path)/include/tikv_client_glue.h


target/debug/libtikv_client.a: target/debug/libtikv_client_rust.a target/debug/tikv_client_glue.o target/debug/tikv_client_cpp.o
	cp $(cur_makefile_path)/target/debug/libtikv_client_rust.a $(cur_makefile_path)/target/debug/libtikv_client.a && ar cr $(cur_makefile_path)/target/debug/libtikv_client.a $(cur_makefile_path)/target/debug/tikv_client_cpp.o $(cur_makefile_path)/target/debug/tikv_client_glue.o

target/debug/tikv_client_cpp.o: src/tikv_client.cpp
	c++ -c $(cur_makefile_path)/src/tikv_client.cpp -o $(cur_makefile_path)/target/debug/tikv_client_cpp.o -std=c++17 -g -I$(cur_makefile_path)/include

target/debug/tikv_client_glue.o: target/tikv_client_glue.cc
	c++ -c $(cur_makefile_path)/target/tikv_client_glue.cc -o $(cur_makefile_path)/target/debug/tikv_client_glue.o -std=c++17

target/debug/libtikv_client_rust.a: src/lib.rs
	cargo build


target/release/libtikv_client.a: target/release/libtikv_client_rust.a target/release/tikv_client_glue.o target/release/tikv_client_cpp.o
	cp $(cur_makefile_path)/target/release/libtikv_client_rust.a $(cur_makefile_path)/target/release/libtikv_client.a && ar cr $(cur_makefile_path)/target/release/libtikv_client.a $(cur_makefile_path)/target/release/tikv_client_cpp.o $(cur_makefile_path)/target/release/tikv_client_glue.o

target/release/tikv_client_cpp.o: src/tikv_client.cpp
	c++ -O3 -c $(cur_makefile_path)/src/tikv_client.cpp -o $(cur_makefile_path)/target/release/tikv_client_cpp.o -std=c++17 -g -I$(cur_makefile_path)/include

target/release/tikv_client_glue.o: target/tikv_client_glue.cc
	c++ -O3 -c $(cur_makefile_path)/target/tikv_client_glue.cc -o $(cur_makefile_path)/target/release/tikv_client_glue.o -std=c++17

target/release/libtikv_client_rust.a: src/lib.rs
	cargo build --release
