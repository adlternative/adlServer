#include "mmap_wrap.hpp"
#include <iostream>
int main(int argc, char **argv) {
  try {
    adl::file_options::mmap mmap_file("a.txt");
  } catch (const std::runtime_error &e) {
    std::cout << e.what() << std::endl;
  }

  try {
    adl::file_options::mmap mmap_file("mmap_lib.cpp");
    std::cout << std::boolalpha << mmap_file.is_open() << std::endl;
    std::cout << std::boolalpha << mmap_file.size() << std::endl;
    std::cout << mmap_file.data() << std::endl;
  } catch (const std::runtime_error &e) {
    std::cout << e.what() << std::endl;
  }
}
