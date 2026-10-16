#include "LandRegistry.hpp"

#include <fstream>
#include <iomanip>
#include <limits>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
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

using Parser = std::unique_ptr<LandPlot> (*)(const std::vector<std::string> &);

struct PlotType {
  size_t minFields;
  Parser parse;
};

const std::map<std::string, PlotType> kPlotTypes = {
    {"R",
     {8,
      [](const std::vector<std::string> &f) -> std::unique_ptr<LandPlot> {
        return std::make_unique<ResidentialPlot>(f[1], std::stod(f[2]), f[3],
                                                 Owner{f[4], f[5], f[6]},
                                                 std::stod(f[7]));
      }}},
    {"A",
     {9,
      [](const std::vector<std::string> &f) -> std::unique_ptr<LandPlot> {
        return std::make_unique<AgriculturalPlot>(f[1], std::stod(f[2]), f[3],
                                                  Owner{f[4], f[5], f[6]}, f[7],
                                                  std::stod(f[8]));
      }}},
    {"C",
     {9,
      [](const std::vector<std::string> &f) -> std::unique_ptr<LandPlot> {
        return std::make_unique<CommercialPlot>(f[1], std::stod(f[2]), f[3],
                                                Owner{f[4], f[5], f[6]}, f[7],
                                                std::stoi(f[8]));
      }}},
};

std::unique_ptr<LandPlot> makePlot(const std::vector<std::string> &f) {
  const auto type = kPlotTypes.find(f[0]);
  if (type == kPlotTypes.end())
    throw std::runtime_error("неизвестный тип участка: " + f[0]);
  if (f.size() < type->second.minFields)
    throw std::runtime_error("для типа '" + f[0] + "' ожидается " +
                             std::to_string(type->second.minFields) +
                             " полей, получено " + std::to_string(f.size()));
  return type->second.parse(f);
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
