#pragma once

#include <ostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "plot/Owner.hpp"

class LandPlot {
public:
  LandPlot(std::string cadastralNumber, double areaSqm, std::string address,
           Owner owner)
      : cadastralNumber_(std::move(cadastralNumber)),
        address_(std::move(address)), owner_(std::move(owner)) {
    setArea(areaSqm);
  }

  virtual ~LandPlot() = default;

  const std::string &cadastralNumber() const { return cadastralNumber_; }
  double area() const { return areaSqm_; }

  void setArea(double areaSqm) {
    if (areaSqm <= 0.0)
      throw std::invalid_argument("площадь участка должна быть положительной");
    areaSqm_ = areaSqm;
  }

  virtual double taxPerSqm() const = 0;
  virtual const char *categoryName() const = 0;
  virtual char typeTag() const = 0;

  virtual double calculateTax() const { return areaSqm_ * taxPerSqm(); }

  virtual void print(std::ostream &os) const {
    os << '[' << categoryName() << "] кад. №" << cadastralNumber_ << ", "
       << address_ << ", " << areaSqm_ << " m2, владелец: " << owner_;
  }

  void serialize(std::ostream &os) const {
    os << typeTag() << ';' << cadastralNumber_ << ';' << areaSqm_ << ';'
       << address_ << ';' << owner_.fullName << ';' << owner_.passportNumber
       << ';' << owner_.phone << ';';
    writeOwn(os);
    os << '\n';
  }

  friend std::ostream &operator<<(std::ostream &os, const LandPlot &plot) {
    plot.print(os);
    return os;
  }

protected:
  virtual void writeOwn(std::ostream &os) const = 0;

private:
  std::string cadastralNumber_;
  double areaSqm_ = 0.0;
  std::string address_;
  Owner owner_;
};
