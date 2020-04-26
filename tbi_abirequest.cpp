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

#include <sys/prctl.h>

int main (int argc, char* argv[]) {
  bool enable = (argc > 1) && (std::atol(argv[1]) > 0);
  if (prctl(PR_SET_TAGGED_ADDR_CTRL,
            enable ? PR_TAGGED_ADDR_ENABLE : 0,
            0, 0, 0) < 0) {
    perror("could not enable TBI");
    return EXIT_FAILURE;
  }
  long result = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  if (result == PR_TAGGED_ADDR_ENABLE) {
    std::cout << "TBI is enabled\n";
  }
  else if (result == 0) {
    std::cout << "TBI is not enabled\n";
  }
  else {
    perror("PR_GET_TAGGED_ADDR_CTRL failed");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

