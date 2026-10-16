#pragma once

#include <ostream>
#include <stdexcept>
#include <utility>

#include "plot/LandPlot.hpp"

class ResidentialPlot final : public LandPlot {
public:
  ResidentialPlot(std::string cadastralNumber, double areaSqm,
                  std::string address, Owner owner, double houseAreaSqm)
      : LandPlot(std::move(cadastralNumber), areaSqm, std::move(address),
                 std::move(owner)),
        houseAreaSqm_(houseAreaSqm) {
    if (houseAreaSqm_ < 0.0)
      throw std::invalid_argument("площадь дома не может быть отрицательной");
  }

  double taxPerSqm() const override { return kTaxRate; }
  const char *categoryName() const override { return "ИЖС"; }
  char typeTag() const override { return 'R'; }

  double calculateTax() const override {
    const double base = LandPlot::calculateTax();
    return houseAreaSqm_ > 0.0 ? base * 0.90 : base;
  }

  void print(std::ostream &os) const override {
    LandPlot::print(os);
    os << ", дом: " << houseAreaSqm_ << "m2"
       << (houseAreaSqm_ > 0.0 ? " (льгота 10%)" : " (без построек)");
  }

private:
  void writeOwn(std::ostream &os) const override { os << houseAreaSqm_; }

  static constexpr double kTaxRate = 0.15;
  double houseAreaSqm_;
};
