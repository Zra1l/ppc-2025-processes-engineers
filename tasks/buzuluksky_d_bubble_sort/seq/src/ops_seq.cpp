#include "buzuluksky_d_bubble_sort/seq/include/ops_seq.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "buzuluksky_d_bubble_sort/common/include/common.hpp"

namespace buzuluksky_d_bubble_sort {

BuzulukskyDBubbleSortSEQ::BuzulukskyDBubbleSortSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  static_cast<void>(GetOutput());
}

bool BuzulukskyDBubbleSortSEQ::ValidationImpl() {
  return true;
}

bool BuzulukskyDBubbleSortSEQ::PreProcessingImpl() {
  return true;
}

bool BuzulukskyDBubbleSortSEQ::RunImpl() {
  const auto &input = GetInput();
  auto arr = input;

  const size_t n = arr.size();
  if (n <= 1) {
    GetOutput() = arr;
    return true;
  }

  bool sorted = false;
  for (size_t pass = 0; pass < n && !sorted; ++pass) {
    sorted = true;

    for (size_t i = 0; i + 1 < n; i += 2) {
      if (arr[i] > arr[i + 1]) {
        std::swap(arr[i], arr[i + 1]);
        sorted = false;
      }
    }

    for (size_t i = 1; i + 1 < n; i += 2) {
      if (arr[i] > arr[i + 1]) {
        std::swap(arr[i], arr[i + 1]);
        sorted = false;
      }
    }

    if (sorted) {
      break;
    }
  }

  GetOutput() = arr;
  return true;
}

bool BuzulukskyDBubbleSortSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace buzuluksky_d_bubble_sort
