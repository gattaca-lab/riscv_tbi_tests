#include <iostream>

#include <sys/prctl.h>

volatile int int_val = 0;
volatile float float_val = 0;
volatile double double_val = 0;

constexpr int ENABLE_TBI = 0xFF;

void tbi_set(uint64_t new_mode, uint64_t& old_status) {
  long result = prctl(PR_GET_TAGGED_ADDR_CTRL, 0, 0, 0, 0);
  old_status = result;
  if (result < 0) {
    perror("could not request TBICONTROL status");
    exit(EXIT_FAILURE);
  }
  if (prctl(PR_SET_TAGGED_ADDR_CTRL,
            (new_mode > 0) ? PR_TAGGED_ADDR_ENABLE : 0,
            0, 0, 0)) {
    perror("could not update TBICONTROL");
    exit(EXIT_FAILURE);
  }
}

bool double_load_check()
{
    double_val = 123.1;
    double valid_res = -1.0;
    double invalid_res = -1.0;
    volatile double* ptr = &double_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "fld %[res], 0(a0)\n"
            : [res] "=f" (valid_res)
            : [in_address] "r" (valid_ptr)
            : "memory");
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "fld %[res], 0(a0)\n"
            : [res] "=f" (invalid_res)
            : [in_address] "r" (invalid_ptr)
            : "memory");
    return (valid_res == invalid_res);
}

bool double_store_check()
{
    double_val = 123.1;
    double val_to_store = 555.0;
    double valid_res = -1.0;
    double invalid_res = -1.0;
    volatile double* ptr = &double_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "fsd %[val], 0(a0)\n"
            :
            : [in_address] "r" (valid_ptr), [val] "f" (val_to_store)
            : "memory");
    valid_res = double_val;
    double_val = 123.1;
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "fsd %[val], 0(a0)\n"
            :
            : [in_address] "r" (invalid_ptr), [val] "f" (val_to_store)
            : "memory");
    invalid_res = double_val;
    return (valid_res == invalid_res);
}

bool float_load_check()
{
    float_val = 123.1f;
    float valid_res = -1.0f;
    float invalid_res = -1.0f;
    volatile float* ptr = &float_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "flw %[res], 0(a0)\n"
            : [res] "=f" (valid_res)
            : [in_address] "r" (valid_ptr)
            : "memory");
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "flw %[res], 0(a0)\n"
            : [res] "=f" (invalid_res)
            : [in_address] "r" (invalid_ptr)
            : "memory");
    return (valid_res == invalid_res);
}

bool float_store_check()
{
    float_val = 123.1f;
    float val_to_store = 555.0f;
    float valid_res = -1.0f;
    float invalid_res = -1.0f;
    volatile float* ptr = &float_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "fsw %[val], 0(a0)\n"
            :
            : [in_address] "r" (valid_ptr), [val] "f" (val_to_store)
            : "memory");
    valid_res = float_val;
    float_val = 123.1f;
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "fsw %[val], 0(a0)\n"
            :
            : [in_address] "r" (invalid_ptr), [val] "f" (val_to_store)
            : "memory");
    invalid_res = float_val;
    return (valid_res == invalid_res);
}

bool int_load_check()
{
    int_val = 123;
    int valid_res = -1;
    int invalid_res = -1;
    volatile int* ptr = &int_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "ld %[res], 0(a0)\n"
            : [res] "=r" (valid_res)
            : [in_address] "r" (valid_ptr)
            : "memory", "a0");
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "ld %[res], 0(a0)\n"
            : [res] "=r" (invalid_res)
            : [in_address] "r" (invalid_ptr)
            : "memory", "a0");
    return (valid_res == invalid_res);
}

bool int_store_check()
{
    int_val = 123;
    int val_to_store = 555;
    int valid_res = -1;
    int invalid_res = -1;
    volatile int* ptr = &int_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "sd %[val], 0(a0)\n"
            :
            : [in_address] "r" (valid_ptr), [val] "r" (val_to_store)
            : "memory", "a0");
    valid_res = int_val;
    int_val = 123;
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "sd %[val], 0(a0)\n"
            :
            : [in_address] "r" (invalid_ptr), [val] "r" (val_to_store)
            : "memory", "a0");
    invalid_res = int_val;
    return (valid_res == invalid_res);
}

bool int_cload_check()
{
    int_val = 123;
    int valid_res = -1;
    int invalid_res = -1;
    volatile int* ptr = &int_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "c.ld %[res], 0(a0)\n"
            : [res] "=r" (valid_res)
            : [in_address] "r" (valid_ptr)
            : "memory", "a0");
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "c.ld %[res], 0(a0)\n"
            : [res] "=r" (invalid_res)
            : [in_address] "r" (invalid_ptr)
            : "memory", "a0");
    return (valid_res == invalid_res);
}

bool int_cstore_check()
{
    int_val = 123;
    int val_to_store = 555;
    int valid_res = -1;
    int invalid_res = -1;
    volatile int* ptr = &int_val;
    uint64_t valid_ptr = (uint64_t)(ptr);
    uint64_t invalid_ptr = valid_ptr | (1ULL << 62);
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "c.sd %[val], 0(a0)\n"
            :
            : [in_address] "r" (valid_ptr), [val] "r" (val_to_store)
            : "memory", "a0");
    valid_res = int_val;
    int_val = 123;
    __asm__  volatile (
            "mv a0, %[in_address]\n"
            "c.sd %[val], 0(a0)\n"
            :
            : [in_address] "r" (invalid_ptr), [val] "r" (val_to_store)
            : "memory", "a0");
    invalid_res = int_val;
    return (valid_res == invalid_res);
}

int main()
{
    // enable tbi
    uint64_t old_status = 0;
    tbi_set(ENABLE_TBI, old_status);

    // check int load
    if (!int_load_check()) {
        std::cout << "Failed to perform int load check" << std::endl;
        exit(-1);
    } else {
        std::cout << "Int load check succeeded" << std::endl;
    }

    // check int store
    if (!int_store_check()) {
        std::cout << "Failed to perform int store check" << std::endl;
        exit(-2);
    } else {
        std::cout << "Int store check succeeded" << std::endl;
    }

    // check float load
    if (!float_load_check()) {
        std::cout << "Failed to perform float load check" << std::endl;
        exit(-3);
    } else {
        std::cout << "Float load check succeeded" << std::endl;
    }

    // check float store
    if (!float_store_check()) {
        std::cout << "Failed to perform float store check" << std::endl;
        exit(-4);
    } else {
        std::cout << "Float store check succeeded" << std::endl;
    }

    // check double load
    if (!double_load_check()) {
        std::cout << "Failed to perform double load check" << std::endl;
        exit(-5);
    } else {
        std::cout << "Double load check succeeded" << std::endl;
    }

    // check double store
    if (!double_store_check()) {
        std::cout << "Failed to perform double store check" << std::endl;
        exit(-6);
    } else {
        std::cout << "Double store check succeeded" << std::endl;
    }

    // check int compressed load
    if (!int_cload_check()) {
        std::cout << "Failed to perform int compressed load check" << std::endl;
        exit(-7);
    } else {
        std::cout << "Int compressed load check succeeded" << std::endl;
    }

    // check int store
    if (!int_cstore_check()) {
        std::cout << "Failed to perform int compressed store check" << std::endl;
        exit(-8);
    } else {
        std::cout << "Int compressed store check succeeded" << std::endl;
    }
    return 0;
}
