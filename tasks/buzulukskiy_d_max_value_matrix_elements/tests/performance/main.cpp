#include <gtest/gtest.h>

#include "buzulukskiy_d_max_value_matrix_elements/common/include/common.hpp"
#include "buzulukskiy_d_max_value_matrix_elements/mpi/include/ops_mpi.hpp"
#include "buzulukskiy_d_max_value_matrix_elements/seq/include/ops_seq.hpp"
#include "util/include/perf_test_util.hpp"

namespace buzulukskiy_d_max_value_matrix_elements {

class BuzulukskiyDMaxValueMatrixElementsPerfTests : public ppc::util::BaseRunPerfTests<InType, OutType> {
 protected:
  void SetUp() override {
    // Создаем большую матрицу для тестирования производительности
    const int rows = 1000;
    const int cols = 1000;
    std::vector<int> data(rows * cols);

    // Заполняем случайными числами, но гарантируем известный максимум
    std::fill(data.begin(), data.end(), 1);
    data[rows * cols - 1] = 99999;  // Известный максимум

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
