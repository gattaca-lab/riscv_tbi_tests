#include <cstdint>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/mman.h>
#include <ucontext.h>
#include <iostream>
#include <thread>
#include <chrono>

#include <sys/ucontext.h>

#define REG_ADDR "0x800"

void tbi_set_broken(uint64_t new_mode, uint64_t& old_status) {
  uint64_t ov = -1;
  __asm__  volatile (
      "csrrw   %[status], " REG_ADDR ", %[new_mode];\n"
      : [status] "=r" (ov)
      : [new_mode] "r" (new_mode)
      : "memory");
  old_status = ov;
}
void tbi_set_working(uint64_t new_mode, uint64_t& old_status, uint64_t& new_status) {
  uint64_t ov = -1;
  uint64_t nv = -1;
  __asm__  volatile (
      "csrr   %[status], " REG_ADDR ";\n"
      "csrw   " REG_ADDR ", %[new_mode];\n"
      "csrr   %[status2], " REG_ADDR ";\n"
      : [status] "=r" (ov), [status2] "=r" (nv)
      : [new_mode] "r" (new_mode)
      : "memory");
  old_status = ov;
  new_status = nv;
}
struct TbiCtrl {
  TbiCtrl(bool update, uint64_t new_mode, bool broken_method) {
    if (!update) {
      std::cerr << "tbictrl - no update\n";
      return;
    }
    if (broken_method) {
      uint64_t status_old = 0;
      tbi_set_broken(new_mode, status_old);
      std::cerr << "requesting new status (2-way method): " << new_mode  << "\n";
    }
    else {
      uint64_t status_old = 0;
      uint64_t status_new = -1;
      std::cerr << "requesting new status (3-way method): " << new_mode  << "\n";
      tbi_set_working(new_mode, status_old, status_new);
      std::cerr << "tbi ctrl updated! old_status: " << status_old <<
                   ", new_status: " << status_new << "\n";
    }
  }
  uint64_t read_tbi() {
    uint64_t status = 0;
    __asm__  volatile (
      "csrr   %[status], " REG_ADDR ";\n"
      : [status] "=r" (status)
      :
      : "memory");
    return status;
  }
};

volatile bool fault_observed = false;
volatile bool g_do_work = true;
volatile bool adjust_a0 = false;
volatile bool expect_fault = false;
volatile int  g_var = 0;

void  the_handler(int signal, siginfo_t * info, void * context) {
     ucontext_t *ucontext = (ucontext_t*)context;
  if (!expect_fault) {
    exit(-6);
  }
  write(0, "...commencing fault handling\n", 29);
  expect_fault = false;
  fault_observed = true;

  if (adjust_a0) {
     ucontext->uc_mcontext.__gregs[10] = (uint64_t)&g_var;
  }
}


int main (int argc, char* argv[]) {
  // install signal handler
  struct sigaction sig_action;
  struct sigaction old_action;
  memset(&sig_action, 0, sizeof(sig_action));

  sig_action.sa_sigaction = the_handler;
  sig_action.sa_flags = SA_RESTART | SA_SIGINFO;
  sigemptyset(&sig_action.sa_mask);

  sigaction(SIGSEGV, &sig_action, &old_action);

  const char *new_status = "0";
  bool update = argc > 1;
  if (update) {
    new_status = argv[1];
  }
  TbiCtrl tbi(update, atol(new_status), argc > 2);
  unsigned i = 0;
  while (g_do_work) {
    ++i;
    uint64_t status = tbi.read_tbi();
    std::cerr << "current tbi status (#" << i << "): " << status << "\n";
    int prev_value = g_var;
    volatile int* ptr = &g_var;
    uint64_t uptr = (uint64_t)ptr;
    uptr |= 1ull << 63;
    if (status) {
      ptr = (volatile int*)uptr;
      ++*ptr;
      if (fault_observed) {
        std::cerr << "fault observed, test FAILED\n";
        exit(-3);
      }
      std::cerr << "checking global variable after update: " << g_var <<
        " (@0x" << std::hex << uptr << std::dec << ")";
      if (prev_value + 1 == g_var) {
        std::cerr << " - passed\n";
      } else {
        std::cerr << " - FAILED\n";
        exit(-2);
      }
    } else {
       adjust_a0 = true;
       expect_fault = true;
       __asm__  volatile (
         "mv a0, %[in_address]\n"
         "ld a1, 0(a0)\n"
         :
         : [in_address] "r" (uptr)
         : "memory", "a1", "a0");

       if (fault_observed) {
         std::cerr << "fault was observed @" << std::hex << uptr << std::dec <<
           " - passed\n";
         fault_observed = false;
       } else {
         std::cerr << "fault is NOT observed, test FAILED\n";
         exit(-4);
       }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  return 0;
}

