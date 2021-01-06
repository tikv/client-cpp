pes_parent_dir:=$(shell pwd)/$(lastword $(MAKEFILE_LIST))
cur_makefile_path := $(shell dirname $(pes_parent_dir))

all: build-lib build-example

build-lib: pre-build target/debug/tikv_client_glue.o target/debug/tikv_client_cpp.o target/debug/libtikv_client.a 

build-example: target/tikv-test 

pre-build: target/debug/libtikv_client_rust.a target/debug/tikv_client_glue.cc include/tikv_client_glue.h

clean:
	cargo clean

run-example: target/tikv-test
	RUST_BACKTRACE=1 $(cur_makefile_path)/target/tikv-test

target/tikv-test: target/debug/libtikv_client.a example/main.cpp
	c++ $(cur_makefile_path)/example/main.cpp -o $(cur_makefile_path)/target/tikv-test -std=c++17 -g -I$(cur_makefile_path)/include -L$(cur_makefile_path)/target/debug -ltikv_client -lpthread -ldl -lssl -lcrypto

target/debug/libtikv_client.a: target/debug/libtikv_client_rust.a target/debug/tikv_client_glue.o target/debug/tikv_client_cpp.o
	cp $(cur_makefile_path)/target/debug/libtikv_client_rust.a $(cur_makefile_path)/target/debug/libtikv_client.a && ar cr $(cur_makefile_path)/target/debug/libtikv_client.a $(cur_makefile_path)/target/debug/tikv_client_cpp.o $(cur_makefile_path)/target/debug/tikv_client_glue.o

target/debug/tikv_client_cpp.o: src/tikv_client.cpp
	c++ -c $(cur_makefile_path)/src/tikv_client.cpp -o $(cur_makefile_path)/target/debug/tikv_client_cpp.o -std=c++17 -g -I$(cur_makefile_path)/include

target/debug/tikv_client_glue.o: target/debug/tikv_client_glue.cc
	c++ -c $(cur_makefile_path)/target/debug/tikv_client_glue.cc -o $(cur_makefile_path)/target/debug/tikv_client_glue.o -std=c++17

target/debug/libtikv_client_rust.a: src/lib.rs
	cargo build

target/debug/tikv_client_glue.cc: src/lib.rs
	cxxbridge $(cur_makefile_path)/src/lib.rs > $(cur_makefile_path)/target/debug/tikv_client_glue.cc

include/tikv_client_glue.h: src/lib.rs
	cxxbridge $(cur_makefile_path)/src/lib.rs --header > $(cur_makefile_path)/include/tikv_client_glue.h
