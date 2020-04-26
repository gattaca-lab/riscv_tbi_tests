# riscv_tbi_tests

## set_tbictrl

The test checks the behavior of a simulator when a tagged memory reference is
detected.

If `atol(argv[1])` is non-zero, then TBI is enabled.
If `atol(argv[1])` is zero, then TBI is disabled.

When TBI is disabled - the test checks that HW generates a fault when a tagged
address is encountered.

When TBI is enabled - the test checks that HW DOES NOT generate a fault when a
tagged address is encountered.

This test has an infinite loop and is expected to be killed by the user with
`Cntrl-C` command.

## tbi_abitrequest

Test targets `prctl` syscall. If `atol(argv[1])` is non-zero, then
the test tries to enable TBI feature and prints result. Otherwise - it tries
to disable it.

## tbi_insn_test

The purpose of this test to ensure that QEMU executes instructions correctly.
As we've modified all instructions which do memory-related operations -
bugs could be introduced. This test tries to detect obvious issues.


# TODO

+ test to check the behavior on fork syscall
+ test to check the behavior on exec syscall

