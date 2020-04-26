# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

RISCV_CC ?= $(shell realpath $(ROOT_DIR)/../../install/bin/riscv64-unknown-linux-gnu-gcc)
RISCV_CXX ?= $(shell realpath  $(ROOT_DIR)/../../install/bin/riscv64-unknown-linux-gnu-g++)
NFS_DIR ?= /tank/work/dev/share/nfs

.PHONY: all

build/set_tbictrl: Makefile set_tbictrl.cpp
	$(RISCV_CXX) -O0 set_tbictrl.cpp -o build/set_tbictrl

build/all: set_tbictrl

deploy: build/set_tbictrl
	cp build/set_tbictrl ${NFS_DIR}/${USER}/tests -r

clean:
	rm build -rf
	mkdir build
	touch build/.keep_me



