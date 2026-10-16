#include "LandRegistry.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "plot/AgriculturalPlot.hpp"
#include "plot/CommercialPlot.hpp"
#include "plot/ResidentialPlot.hpp"

namespace {

std::vector<std::string> splitFields(const std::string &line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;
  while (std::getline(stream, field, ';'))
    fields.push_back(field);
  return fields;
}

std::unique_ptr<LandPlot> makePlot(const std::vector<std::string> &f) {
  if (f.size() < 8)
    throw std::runtime_error("ожидается минимум 8 полей, получено " +
                             std::to_string(f.size()));

  Owner owner{f[4], f[5], f[6]};
  if (f[0] == "R")
    return std::make_unique<ResidentialPlot>(f[1], std::stod(f[2]), f[3],
                                             std::move(owner), std::stod(f[7]));
  if (f[0] == "A") {
    if (f.size() < 9)
      throw std::runtime_error("для сельхозучастка ожидается 9 полей");
    return std::make_unique<AgriculturalPlot>(
        f[1], std::stod(f[2]), f[3], std::move(owner), f[7], std::stod(f[8]));
  }
  if (f[0] == "C") {
    if (f.size() < 9)
      throw std::runtime_error("для коммерческого участка ожидается 9 полей");
    return std::make_unique<CommercialPlot>(
        f[1], std::stod(f[2]), f[3], std::move(owner), f[7], std::stoi(f[8]));
  }
  throw std::runtime_error("неизвестный тип участка: " + f[0]);
}

} // namespace

void LandRegistry::save(const std::string &path) const {
  std::ofstream out(path);
  if (!out.is_open())
    throw std::runtime_error("не удалось открыть файл для записи: " + path);

  out << std::setprecision(std::numeric_limits<double>::max_digits10);
  forEach([&out](const LandPlot &plot) { plot.serialize(out); });

  out.flush();
  if (!out.good())
    throw std::runtime_error("ошибка записи в файл: " + path);
}

void LandRegistry::load(const std::string &path) {
  std::ifstream in(path);
  if (!in.is_open())
    throw std::runtime_error("не удалось открыть файл: " + path);

  std::string line;
  int lineNumber = 0;
  while (std::getline(in, line)) {
    ++lineNumber;
    if (line.empty() || line[0] == '#')
      continue;
    try {
      add(makePlot(splitFields(line)));
    } catch (const std::exception &e) {
      throw std::runtime_error(path + ", строка " + std::to_string(lineNumber) +
                               ": " + e.what());
    }
  }
}
