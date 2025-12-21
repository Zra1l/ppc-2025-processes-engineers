#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iostream>
#include <vector>

#include "buzuluksky_d_bubble_sort/mpi/include/ops_mpi.hpp"
#include "buzuluksky_d_bubble_sort/seq/include/ops_seq.hpp"

namespace buzuluksky_d_bubble_sort {

static bool IsSorted(const std::vector<int> &arr) {
  if (arr.empty() || arr.size() == 1) {
    return true;
  }

  for (std::size_t i = 0; i + 1 < arr.size(); ++i) {
    if (arr[i] > arr[i + 1]) {
      return false;
    }
  }
  return true;
}

class BubbleSortPerfTest : public ::testing::Test {
 protected:
  BubbleSortPerfTest() : size_(5000) {}

  void SetUp() override {
    input_.resize(size_);
    for (std::size_t i = 0; i < size_; ++i) {
      input_[i] = static_cast<int>(i);
      if (i % 100 == 0) {
        input_[i] = static_cast<int>((i * 37) % size_);
      }
    }

    if (size_ > 10) {
      std::swap(input_[0], input_[size_ - 1]);
      std::swap(input_[size_ / 4], input_[size_ / 2]);
    }
  }

  std::vector<int> input_;
  std::size_t size_;
};

TEST_F(BubbleSortPerfTest, SeqPerformance) {
  BuzulukskyDBubbleSortSEQ task(input_);

  const auto start = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());

  const auto end = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  const auto result = task.GetOutput();
  EXPECT_TRUE(IsSorted(result));
  std::cout << "SEQ execution time: " << duration.count() << " ms\n";
}

TEST_F(BubbleSortPerfTest, MpiPerformance) {
  BuzulukskyDBubbleSortMPI task(input_);

  const auto start = std::chrono::high_resolution_clock::now();

  EXPECT_TRUE(task.Validation());
  EXPECT_TRUE(task.PreProcessing());
  EXPECT_TRUE(task.Run());
  EXPECT_TRUE(task.PostProcessing());

  const auto end = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  const auto result = task.GetOutput();
  EXPECT_TRUE(IsSorted(result));
  std::cout << "MPI execution time: " << duration.count() << " ms\n";
}

}  // namespace buzuluksky_d_bubble_sort
