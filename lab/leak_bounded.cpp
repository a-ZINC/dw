#include <chrono>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char** argv) {
    int iterations = (argc > 1) ? std::atoi(argv[1]) : 10;
    for (int i=0; i<iterations; i++) {
        char* x = static_cast<char*>(malloc(1024 * 1024));
        if (!x) {
            return 1;
        }

        x[0] = 'x';
        std::cout << "iter: " << i <<std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    return 0;
}