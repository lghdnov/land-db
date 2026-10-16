#pragma once

#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "plot/LandPlot.hpp"

class CommercialPlot final : public LandPlot {
public:
  CommercialPlot(std::string cadastralNumber, double areaSqm,
                 std::string address, Owner owner, std::string purpose,
                 int floors)
      : LandPlot(std::move(cadastralNumber), areaSqm, std::move(address),
                 std::move(owner)),
        purpose_(std::move(purpose)), floors_(floors) {
    if (floors_ < 1)
      throw std::invalid_argument("этажность должна быть не меньше 1");
  }

  double taxPerSqm() const override {
    return kBaseRate + kPerFloorSurcharge * floors_;
  }
  const char *categoryName() const override { return "Коммерческий"; }
  char typeTag() const override { return 'C'; }

  void print(std::ostream &os) const override {
    LandPlot::print(os);
    os << ", назначение: " << purpose_ << ", этажей: " << floors_;
  }

private:
  void writeOwn(std::ostream &os) const override {
    os << purpose_ << ';' << floors_;
  }

  static constexpr double kBaseRate = 1.2;
  static constexpr double kPerFloorSurcharge = 0.1;
  std::string purpose_;
  int floors_;
};
