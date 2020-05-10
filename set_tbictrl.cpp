#include <cstdint>
#include <cstdlib>
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
#include <cassert>
#include <thread>
#include <chrono>

#include <sys/ucontext.h>
#include <sys/prctl.h>

void tbi_set_working(uint64_t new_mode, uint64_t& old_status, uint64_t& new_status) {
  long result = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  if (result < 0) {
    perror("could not get TBI status");
    exit(EXIT_FAILURE);
  }
  old_status = result;
  if (prctl(PR_SET_TAGGED_ADDR_CTRL,
            (new_mode > 0) ? PR_TAGGED_ADDR_ENABLE : 0,
            0, 0, 0) < 0) {
    perror("could not update TBI");
    exit(EXIT_FAILURE);
  }
#if 0
// This part is for debugging only, and won't work once REG_ADDR becomes priviledged
#define REG_ADDR "0x800"
  if (new_mode > 0) {
    uint64_t status = 0;
    __asm__  volatile (
    "csrr   %[status], " REG_ADDR ";\n"
    : [status] "=r" (status)
    :
    : "memory");
    std::cerr << "the status of TBICTRL: " << status << "\n";
  }
#endif

  result = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  if (result < 0) {
    perror("could not get TBI status");
    exit(EXIT_FAILURE);
  }
  new_status = result;
}
struct TbiCtrl {
  TbiCtrl(bool update, uint64_t new_mode) {
    if (!update) {
      std::cerr << "tbictrl - no update\n";
      return;
    }
    uint64_t status_old = 0;
    uint64_t status_new = -1;
    std::cerr << "requesting new status: " << new_mode  << "\n";
    tbi_set_working(new_mode, status_old, status_new);
    std::cerr << "tbi ctrl updated! old_status: " << status_old <<
                 ", new_status: " << status_new << "\n";
  }
  uint64_t read_tbi() {
    long result = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
    if (result < 0) {
      perror("could not get TBI status");
      exit(EXIT_FAILURE);
    }
    return result;
  }
};

volatile bool fault_observed = false;
volatile bool g_do_work = true;
volatile bool adjust_a0 = false;
volatile bool expect_fault = false;
volatile int  g_var = 0;

void sys_write(const char* str) {
  write(0, str, strlen(str));
}
void  the_handler(int signal, siginfo_t * info, void * context) {
     ucontext_t *ucontext = (ucontext_t*)context;
  if (!expect_fault) {
    sys_write("unexpected fault detected\n");
    exit(-6);
  }
  sys_write("...commencing fault handling\n");

  expect_fault = false;
  fault_observed = true;

  if (adjust_a0) {
     ucontext->uc_mcontext.__gregs[10] = (uint64_t)&g_var;
  }
}

#define D_TAGGED_OP_CHECK(opcode, addr) \
  sys_write("checking <" #opcode "> operation...\n"); \
    __asm__  volatile (                               \
        "mv a0, %[in_address]\n"                      \
        #opcode " a1, 0(a0)\n"                        \
        :                                             \
        : [in_address] "r" (addr)                     \
        : "memory", "a1", "a0")                       \

void test_operation(uint64_t uptr) {

  struct Seed { Seed(unsigned seed) { std::srand(seed); } };

  static Seed TheSeed(1);

  switch(std::rand() % 7) {
  case 0: D_TAGGED_OP_CHECK(ld, uptr); break;
  case 1: D_TAGGED_OP_CHECK(lb, uptr); break;
  case 2: D_TAGGED_OP_CHECK(lbu, uptr); break;
  case 3: D_TAGGED_OP_CHECK(sd, uptr); break;
  case 4: D_TAGGED_OP_CHECK(sb, uptr); break;
  case 5: D_TAGGED_OP_CHECK(sw, uptr); break;
  case 6: D_TAGGED_OP_CHECK(lw, uptr); break;
  default:
    sys_write("should not happend - unexpected type of check requested!\n");
    exit(-100);
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
  TbiCtrl tbi(update, atol(new_status));
  unsigned i = 0;
  while (g_do_work) {
    ++i;
    uint64_t status = tbi.read_tbi();
    std::cerr << "current tbi status (#" << i << "): " << status << "\n";
    int prev_value = g_var;
    volatile int* ptr = &g_var;
    uint64_t uptr = (uint64_t)ptr;
    uint64_t tag = (i % 255u) + 1u;
    uptr |= tag << 56;
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

       test_operation(uptr);

       if (fault_observed) {
         std::cerr << "fault was observed @" << std::hex << uptr << std::dec <<
           " - passed\n";
         fault_observed = false;
       } else {
         std::cerr << "fault is NOT observed, test FAILED\n";
         exit(-4);
       }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }
  return 0;
}

