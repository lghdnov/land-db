#pragma once

#include <iosfwd>
#include <string>
#include <vector>

#include "third_party/CLI11.hpp"
#include "registry/LandRegistry.hpp"

class TerminalApp {
public:
  explicit TerminalApp(LandRegistry registry = {})
      : registry_(std::move(registry)) {}

  void loadDatabase(std::ostream &out);
  void run(std::istream &in, std::ostream &out);
  int execute(const std::vector<std::string> &tokens, std::ostream &out);
  void saveDatabase(std::ostream &out);

private:
  struct Args {
    std::string cadastral, address, fullName, passport, phone = "-",
                                                        crop, purpose;
    double area = 0.0, houseArea = 0.0, fertility = 1.0;
    double newArea = 0.0, newFertility = 1.0;
    int floors = 1;
  };

  void configure(CLI::App &app, Args &args);
  void addCommandsAdd(CLI::App &app, Args &args);
  void addCommandsView(CLI::App &app, Args &args);
  void addCommandsEdit(CLI::App &app, Args &args);
  void addCommonOptions(CLI::App *sub, Args &args);
  void seedDemoData();
  static std::vector<std::string> tokenize(const std::string &line);

  LandRegistry registry_;
  bool running_ = true;
  bool dirty_ = false;
  std::ostream *out_ = nullptr;
};
