#pragma once

#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "plot/LandPlot.hpp"

class AgriculturalPlot final : public LandPlot {
public:
  AgriculturalPlot(std::string cadastralNumber, double areaSqm,
                   std::string address, Owner owner, std::string crop,
                   double soilFertility)
      : LandPlot(std::move(cadastralNumber), areaSqm, std::move(address),
                 std::move(owner)),
        crop_(std::move(crop)) {
    setSoilFertility(soilFertility);
  }

  double taxPerSqm() const override { return kTaxRate; }
  const char *categoryName() const override { return "Сельхоз"; }
  char typeTag() const override { return 'A'; }

  double calculateTax() const override {
    return area() * taxPerSqm() * soilFertility_;
  }

  void setSoilFertility(double value) {
    if (value < 0.1 || value > 1.0)
      throw std::invalid_argument(
          "коэффициент плодородия должен быть в диапазоне [0.1, 1.0]");
    soilFertility_ = value;
  }

  void print(std::ostream &os) const override {
    LandPlot::print(os);
    os << ", культура: " << crop_ << ", плодородие: " << soilFertility_;
  }

private:
  void writeOwn(std::ostream &os) const override {
    os << crop_ << ';' << soilFertility_;
  }

  static constexpr double kTaxRate = 0.05;
  std::string crop_;
  double soilFertility_ = 1.0;
};
