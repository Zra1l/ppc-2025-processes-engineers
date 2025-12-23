#include "buzulukskiy_d_sort_batcher/seq/include/ops_seq.hpp"

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <iterator>
#include <utility>
#include <vector>

namespace buzulukskiy_d_sort_batcher {

namespace {
constexpr int kRadixBase = 10;
constexpr std::size_t kBlockSize = 64;

void RadixSortUnsigned(std::vector<int> &arr) {
  if (arr.empty()) {
    return;
  }

  const int max_val = *std::ranges::max_element(arr);
  std::vector<int> output(arr.size());

  for (int exp = 1; max_val / exp > 0; exp *= kRadixBase) {
    std::array<int, kRadixBase> count = {0};

    for (const int value : arr) {
      ++count[(value / exp) % kRadixBase];
    }

    for (int i = 1; i < kRadixBase; ++i) {
      count[i] += count[i - 1];
    }

    for (std::size_t i = arr.size(); i-- > 0;) {
      const int digit = (arr[i] / exp) % kRadixBase;
      output[--count[digit]] = arr[i];
    }

    arr.swap(output);
  }
}

void RadixSortLSD(std::vector<int> &data) {
  if (data.empty()) {
    return;
  }

  std::vector<int> positives;
  std::vector<int> negatives;

  for (const int value : data) {
    if (value < 0) {
      if (value == INT_MIN) {
        negatives.push_back(INT_MAX);
      } else {
        negatives.push_back(-value);
      }
    } else {
      positives.push_back(value);
    }
  }

  if (!positives.empty()) {
    RadixSortUnsigned(positives);
  }

  if (!negatives.empty()) {
    RadixSortUnsigned(negatives);
    std::ranges::reverse(negatives);
    for (int &value : negatives) {
      if (value == INT_MAX) {
        value = INT_MIN;
      } else {
        value = -value;
      }
    }
  }

  data.clear();
  data.insert(data.end(), negatives.begin(), negatives.end());
  data.insert(data.end(), positives.begin(), positives.end());
}

std::vector<int> BatcherOddEvenMerge(const std::vector<int> &a, const std::vector<int> &b) {
  std::vector<int> result;
  result.reserve(a.size() + b.size());
  std::ranges::merge(a, b, std::back_inserter(result));
  return result;
}

}  // namespace

BuzulukskiyDSortBatcherSEQ::BuzulukskiyDSortBatcherSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool BuzulukskiyDSortBatcherSEQ::ValidationImpl() {
  return true;
}

bool BuzulukskiyDSortBatcherSEQ::PreProcessingImpl() {
  return true;
}

bool BuzulukskiyDSortBatcherSEQ::PostProcessingImpl() {
  return true;
}

bool BuzulukskiyDSortBatcherSEQ::RunImpl() {
  const std::vector<int> &input = GetInput();
  if (input.size() <= 1) {
    GetOutput().clear();
    if (!input.empty()) {
      GetOutput().push_back(input[0]);
    }
    return true;
  }

  std::vector<std::vector<int>> blocks;
  blocks.reserve((input.size() + kBlockSize - 1) / kBlockSize);

  for (std::size_t i = 0; i < input.size(); i += kBlockSize) {
    const std::size_t end = std::min(i + kBlockSize, input.size());
    blocks.emplace_back(input.begin() + static_cast<std::ptrdiff_t>(i),
                        input.begin() + static_cast<std::ptrdiff_t>(end));
  }

  for (auto &block : blocks) {
    if (!block.empty()) {
      RadixSortLSD(block);
    }
  }

  while (blocks.size() > 1) {
    std::vector<std::vector<int>> next_blocks;
    next_blocks.reserve((blocks.size() + 1) / 2);

    for (std::size_t i = 0; i + 1 < blocks.size(); i += 2) {
      next_blocks.push_back(BatcherOddEvenMerge(blocks[i], blocks[i + 1]));
    }

    if (blocks.size() % 2 == 1) {
      next_blocks.push_back(std::move(blocks.back()));
    }

    blocks.swap(next_blocks);
  }

  if (blocks.size() == 1 && !blocks[0].empty()) {
    GetOutput() = std::vector<int>(blocks[0].begin(), blocks[0].end());
  } else {
    GetOutput() = std::vector<int>();
  }

  return true;
}

}  // namespace buzulukskiy_d_sort_batcher
