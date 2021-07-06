#include "acceptor.hpp"
#include <fmt/color.h>
#include <fmt/core.h>

int main(int argc, char const *argv[]) {
  adl::acceptor::Acceptor accector("127.0.0.1", 12345);
  accector.temp_poller();
}