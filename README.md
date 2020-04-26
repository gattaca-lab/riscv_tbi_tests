# riscv_tbi_tests

## set_tbictrl

Tests checkes the behaviour of the simulator when tagged memory reference is
detected.

If `atol(argv[1])` is non-zero, then TBI is enabled.
If `atol(argv[1])` is zero, then TBI is disabled.

When TBI is disabled - the test checks that HW generates a fault when tagged
pointer is encountered.

When TBI is enabled - the test checks that HW DOES NOT generate a fault
when tagged pointer is encountered.

This test has an infinite loop and is expected to be killed by user with
`Cntrl-C` command.

## tbi_abitrequest

Test targets `prctl` syscall. If `atol(argv[1])` is non-zero, then
tests tries to enable TBI feature and prints result. Otherwise - it tries
to disable it.

## tbi_insn_test

The purpose of this test to ensure that QEMU executes instructions correctly.
As we've modified all instructions which do memory-related operations -
bugs could be introduced. This tests tries to detect obvious issues.


# TODO

+ test to check the behavior on fork
+ test to check the behavior on exec

