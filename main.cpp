#include <iostream>
#include <vector>

#include "features/terminal/TerminalApp.hpp"

int main(int argc, char **argv) {
  TerminalApp terminal;

  if (argc > 1) {
    terminal.loadDatabase(std::cout);
    const int code = terminal.execute({argv + 1, argv + argc}, std::cout);
    terminal.saveDatabase(std::cout);
    return code;
  }

  terminal.run(std::cin, std::cout);
  return 0;
}
