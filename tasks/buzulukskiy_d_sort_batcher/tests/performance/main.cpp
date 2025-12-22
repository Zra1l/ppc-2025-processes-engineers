#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <vector>

#include "buzulukskiy_d_sort_batcher/common/include/common.hpp"
#include "buzulukskiy_d_sort_batcher/mpi/include/ops_mpi.hpp"
#include "buzulukskiy_d_sort_batcher/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace buzulukskiy_d_sort_batcher {

class BuzulukskiyDSortBatcherPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    input_data_.resize(kCount);
    for (std::size_t i = 0; i < kCount; ++i) {
      input_data_[i] = static_cast<int>((kCount - i) * 7 % 10000 - 5000);
    }
  }

  InType GetTestInputData() final {
    return input_data_;
  }

  bool CheckTestOutputData(OutType &output) final {
    std::vector<int> expected = input_data_;
    std::sort(expected.begin(), expected.end());
    return output == expected;
  }

 private:
  static constexpr std::size_t kCount = 500000;
  InType input_data_;
};

TEST_P(BuzulukskiyDSortBatcherPerfTests, RunPerf) {
  const auto start = std::chrono::high_resolution_clock::now();
  ExecuteTest(GetParam());
  const auto end = std::chrono::high_resolution_clock::now();

  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  std::cout << "Execution time: " << ms << " ms\n";
}

const auto kPerfTasks = ppc::util::MakeAllPerfTasks<InType, BuzulukskiyDSortBatcherSEQ, BuzulukskiyDSortBatcherMPI>(
    PPC_SETTINGS_buzulukskiy_d_sort_batcher);

const auto kPerfValues = ppc::util::TupleToGTestValues(kPerfTasks);

INSTANTIATE_TEST_SUITE_P(SortBatcherPerfTests, BuzulukskiyDSortBatcherPerfTests, kPerfValues,
                         BuzulukskiyDSortBatcherPerfTests::CustomPerfTestName);

}  // namespace buzulukskiy_d_sort_batcher
