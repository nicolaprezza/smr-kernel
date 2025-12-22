#include <iostream>
#include <vector>
#include <fstream>
#include <cstdint>
#include "libsais.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "Error: Could not open file." << std::endl;
        return 1;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        std::cerr << "Error: Could not read file." << std::endl;
        return 1;
    }

    std::vector<int32_t> SA(size);

    std::cout << "Building Suffix Array for " << size << " bytes..." << std::endl;

    int32_t result = libsais(buffer.data(), SA.data(), (int32_t)size, 0, nullptr);

    if (result >= 0) {
        std::cout << "Success! Suffix Array constructed." << std::endl;
    } else {
        std::cerr << "Error: Suffix Array construction failed." << std::endl;
        return 1;
    }

    return 0;
}
