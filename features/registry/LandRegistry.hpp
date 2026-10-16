#pragma once

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "plot/LandPlot.hpp"

class LandRegistry {
public:
  void add(std::unique_ptr<LandPlot> plot) {
    if (plot == nullptr)
      throw std::invalid_argument("реестр: попытка добавить пустой участок");
    if (find(plot->cadastralNumber()) != nullptr)
      throw std::invalid_argument("участок номером " + plot->cadastralNumber() +
                                  " уже зарегистрирован");
    plots_.push_back(std::move(plot));
  }

  bool remove(const std::string &cadastralNumber) {
    const auto it =
        std::find_if(plots_.begin(), plots_.end(),
                     [&cadastralNumber](const std::unique_ptr<LandPlot> &plot) {
                       return plot->cadastralNumber() == cadastralNumber;
                     });
    if (it == plots_.end())
      return false;
    plots_.erase(it);
    return true;
  }

  LandPlot *find(const std::string &cadastralNumber) const {
    const auto it =
        std::find_if(plots_.begin(), plots_.end(),
                     [&cadastralNumber](const std::unique_ptr<LandPlot> &plot) {
                       return plot->cadastralNumber() == cadastralNumber;
                     });
    return it == plots_.end() ? nullptr : it->get();
  }

  double totalTax() const {
    double sum = 0.0;
    for (const auto &plot : plots_)
      sum += plot->calculateTax();
    return sum;
  }

  size_t size() const { return plots_.size(); }

  template <typename Visitor> void forEach(Visitor &&visit) const {
    for (const auto &plot : plots_)
      visit(*plot);
  }

  void save(const std::string &path) const;
  void load(const std::string &path);

private:
  std::vector<std::unique_ptr<LandPlot>> plots_;
};
