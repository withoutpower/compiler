#include <iostream>
#include "ast.h"

void print_operator(const parameter &p1, const parameter &p2) {
  if(p1.is_data == 0) {
    std::cout << "%" << p1.p1 << ", ";
  }
  else {
    std::cout << p1.p1 << ", ";
  }

  if(p2.is_data == 0) {
    std::cout << "%" << p2.p1 << std::endl;
  }
  else {
    std::cout << p2.p1 << std::endl;
  }

}
