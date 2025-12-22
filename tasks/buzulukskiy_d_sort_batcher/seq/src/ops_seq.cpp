#include "buzulukskiy_d_sort_batcher/seq/include/ops_seq.hpp"

#include <algorithm>
#include <climits>
#include <cstddef>
#include <iterator>
#include <vector>

namespace buzulukskiy_d_sort_batcher {

namespace {
constexpr int kRadixBase = 10;
constexpr std::size_t kBlockSize = 64;

void RadixSortUnsigned(std::vector<int> &arr) {
  if (arr.empty()) {
    return;
  }

  const int maxVal = *std::max_element(arr.begin(), arr.end());
  std::vector<int> output(arr.size());

  for (int exp = 1; maxVal / exp > 0; exp *= kRadixBase) {
    int count[kRadixBase] = {0};

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

    std::reverse(negatives.begin(), negatives.end());
    for (int &value : negatives) {
      if (value == INT_MAX) {
        value = INT_MIN;
      } else {
        value = -value;
      }
    }
  }

  data.clear();
  if (!negatives.empty()) {
    data.insert(data.end(), negatives.begin(), negatives.end());
  }
  if (!positives.empty()) {
    data.insert(data.end(), positives.begin(), positives.end());
  }
}

std::vector<int> BatcherOddEvenMerge(const std::vector<int> &a, const std::vector<int> &b) {
  std::vector<int> result;
  result.reserve(a.size() + b.size());
  std::merge(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
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
    GetOutput() = input;
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
    std::vector<std::vector<int>> nextBlocks;
    nextBlocks.reserve((blocks.size() + 1) / 2);

    for (std::size_t i = 0; i + 1 < blocks.size(); i += 2) {
      nextBlocks.push_back(BatcherOddEvenMerge(blocks[i], blocks[i + 1]));
    }

    if (blocks.size() % 2 == 1) {
      nextBlocks.push_back(std::move(blocks.back()));
    }

    blocks.swap(nextBlocks);
  }

  if (!blocks.empty()) {
    GetOutput() = std::move(blocks.front());
  } else {
    GetOutput() = InType();
  }

  return true;
}

}  // namespace buzulukskiy_d_sort_batcher
