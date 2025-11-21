#include <gtest/gtest.h>

#include <random>

#include "buzulukskiy_d_max_value_matrix_elements/common/include/common.hpp"
#include "buzulukskiy_d_max_value_matrix_elements/mpi/include/ops_mpi.hpp"
#include "buzulukskiy_d_max_value_matrix_elements/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace buzulukskiy_d_max_value_matrix_elements {

class BuzulukskiyDMaxValueMatrixElementsPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    const int rows = 200;
    const int cols = 200;
    const int matrix_size = rows * cols;
    std::vector<int> data(matrix_size);

    std::mt19937 gen(42);
    std::uniform_int_distribution<int> dist(1, 100000);

    for (int i = 0; i < rows * cols; i++) {
      data[i] = dist(gen);
    }

    data[0] = 99999;
    data[matrix_size / 4] = 99998;
    data[matrix_size / 2] = 99997;
    data[3 * matrix_size / 4] = 99996;
    data[matrix_size - 1] = 99999;

    input_data_.rows = rows;
    input_data_.columns = cols;
    input_data_.data = data;
    expected_max_ = 99999;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_max_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 private:
  InType input_data_{};
  int expected_max_ = 0;
};

TEST_P(BuzulukskiyDMaxValueMatrixElementsPerfTests, RunPerfModes) {
  ExecuteTest(GetParam());
}

const auto kAllPerfTasks =
    ppc::util::MakeAllPerfTasks<InType, BuzulukskiyDMaxValueMatrixElementsMPI, BuzulukskiyDMaxValueMatrixElementsSEQ>(
        PPC_SETTINGS_buzulukskiy_d_max_value_matrix_elements);

const auto kGtestValues = ppc::util::TupleToGTestValues(kAllPerfTasks);

const auto kPerfTestName = BuzulukskiyDMaxValueMatrixElementsPerfTests::CustomPerfTestName;

INSTANTIATE_TEST_SUITE_P(RunModeTests, BuzulukskiyDMaxValueMatrixElementsPerfTests, kGtestValues, kPerfTestName);

}  // namespace buzulukskiy_d_max_value_matrix_elements
