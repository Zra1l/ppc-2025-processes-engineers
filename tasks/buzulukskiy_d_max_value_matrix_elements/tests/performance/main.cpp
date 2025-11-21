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
    const int rows = 4000;
    const int cols = 4000;
    const int matrix_size = rows * cols;
    std::vector<int> data(matrix_size);

    for (int index = 0; index < matrix_size; ++index) {
      data[index] = (index % 10000) + 1;  // 1-10000
    }

    data[0] = 99999;
    data[matrix_size - 1] = 99999;

    input_data.rows = rows;
    input_data.columns = cols;
    input_data.data = data;
    expected_max = 99999;
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_max == output_data);
  }

  InType GetTestInputData() final {
    return input_data;
  }

 private:
  InType input_data{};
  int expected_max = 0;
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
