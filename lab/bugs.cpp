// The bug zoo: four deliberate bugs selected by argument.
#include <climits>
#include <cstdio>
#include <cstring>

static void leak() {
    char* p = new char[100];
    p[0] = 1;
    std::printf("leak: %d\n", p[0]);
    // never delete[]
}

static void overflow() {
    int* a = new int[4];
    a[100000] = 1;  // one past the end
    std::printf("overflow: %d\n", a[100]);
    delete[] a;
}

static void use_after_free() {
    int* a = new int[4];
    a[0] = 7;
    delete[] a;
    std::printf("uaf: %d\n", a[0]);  // reads freed memory
}

static void ub_overflow() {
    volatile int x = INT_MAX;
    int y = x + 1;  // signed overflow: undefined behavior
    std::printf("ub: %d\n", y);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: bugs leak|overflow|uaf|ub\n");
        return 2;
    }
    if (std::strcmp(argv[1], "leak") == 0) {
        leak();
    } else if (std::strcmp(argv[1], "overflow") == 0) {
        overflow();
    } else if (std::strcmp(argv[1], "uaf") == 0) {
        use_after_free();
    } else if (std::strcmp(argv[1], "ub") == 0) {
        ub_overflow();
    } else {
        std::fprintf(stderr, "unknown bug: %s\n", argv[1]);
        return 2;
    }
    return 0;
}