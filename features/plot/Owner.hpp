#pragma once

#include <ostream>
#include <string>

struct Owner {
  std::string fullName;
  std::string passportNumber;
  std::string phone = "-";

  friend std::ostream &operator<<(std::ostream &os, const Owner &owner) {
    return os << owner.fullName << " (паспорт " << owner.passportNumber
              << ", тел. " << owner.phone << ')';
  }
};
