# https://stackoverflow.com/questions/18136918/how-to-get-current-relative-directory-of-your-makefile
ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

RISCV_CC ?= $(shell realpath $(ROOT_DIR)/../../install/bin/riscv64-unknown-linux-gnu-gcc)
RISCV_CXX ?= $(shell realpath  $(ROOT_DIR)/../../install/bin/riscv64-unknown-linux-gnu-g++)
NFS_DIR ?= /tank/work/dev/share/nfs

DEPLOY_DIR ?= ${NFS_DIR}/${USER}/tests

CTRL_TEST = set_tbictrl
ABI_TEST = tbi_abirequest
TBI_INSN_TEST = tbi_insn_test

.PHONY: all

build/$(CTRL_TEST): Makefile $(CTRL_TEST).cpp
	$(RISCV_CXX) -O0 $(CTRL_TEST).cpp -o build/$(CTRL_TEST)

build/$(ABI_TEST): Makefile $(ABI_TEST).cpp
	$(RISCV_CXX) -O0 $(ABI_TEST).cpp -o build/$(ABI_TEST)

build/$(TBI_INSN_TEST): Makefile $(TBI_INSN_TEST).cpp
	$(RISCV_CXX) -O0 test_tbi.cpp -o build/$(TBI_INSN_TEST)

all: \
	build/$(CTRL_TEST) \
	build/$(ABI_TEST) \
	build/$(TBI_INSN_TEST)

deploy: all
	cp build/$(CTRL_TEST)     -r $(DEPLOY_DIR)
	cp build/$(ABI_TEST)      -r $(DEPLOY_DIR)
	cp build/$(TBI_INSN_TEST) -r $(DEPLOY_DIR)

clean:
	rm build -rf
	mkdir build
	touch build/.keep_me



