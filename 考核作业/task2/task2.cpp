
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    std::cout << "TASK2 C++ program is running.\n";

    std::cout << "Argument count (excluding program name): "
              << argc - 1 << '\n';

    for (int i = 1; i < argc; ++i) {
        std::cout << "argv[" << i << "] = "
                  << argv[i] << '\n';
    }

    if (argc == 1) {
        std::cout << "No command-line arguments were provided.\n";
    }

    return 0;
}
