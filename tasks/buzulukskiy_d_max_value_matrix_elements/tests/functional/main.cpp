#include <gtest/gtest.h>

#include <array>
#include <cstddef>
#include <string>
#include <tuple>
#include <vector>

#include "buzulukskiy_d_max_value_matrix_elements/common/include/common.hpp"
#include "buzulukskiy_d_max_value_matrix_elements/mpi/include/ops_mpi.hpp"
#include "buzulukskiy_d_max_value_matrix_elements/seq/include/ops_seq.hpp"
#include "util/include/func_test_util.hpp"

namespace buzulukskiy_d_max_value_matrix_elements {

class BuzulukskiyDMaxValueMatrixElementsTests : public ppc::util::BaseRunFuncTests<InType, OutType, TestType> {
 protected:
  void SetUp() override {
    auto params = std::get<static_cast<std::size_t>(ppc::util::GTestParamIndex::kTestParams)>(GetParam());
    test_case_ = std::get<0>(params);

    switch (test_case_) {
      case 1:
        input_data_.rows = 2;
        input_data_.columns = 2;
        input_data_.data = {1, 2, 3, 4};
        expected_max_ = 4;
        break;
      case 2:
        input_data_.rows = 3;
        input_data_.columns = 2;
        input_data_.data = {-1, -5, 8, -3, 0, 7};
        expected_max_ = 8;
        break;
      case 3:
        input_data_.rows = 1;
        input_data_.columns = 1;
        input_data_.data = {42};
        expected_max_ = 42;
        break;
      case 4:
        input_data_.rows = 2;
        input_data_.columns = 3;
        input_data_.data = {5, 5, 5, 5, 10, 5};
        expected_max_ = 10;
        break;
      default:
        throw std::runtime_error("Unknown test case");
    }
  }

  bool CheckTestOutputData(OutType &output_data) final {
    return (expected_max_ == output_data);
  }

  InType GetTestInputData() final {
    return input_data_;
  }

 public:
  static std::string PrintTestParam(const TestType &test_param) {
    return "TestCase_" + std::to_string(std::get<0>(test_param)) + "_" + std::get<1>(test_param);
  }

 private:
  InType input_data_{};
  int expected_max_ = 0;
  int test_case_ = 0;
};

namespace {

TEST_P(BuzulukskiyDMaxValueMatrixElementsTests, MatmulFromPic) {
  ExecuteTest(GetParam());
}

const std::array<TestType, 4> kTestParam = {std::make_tuple(1, "2x2_matrix"), std::make_tuple(2, "with_negatives"),
                                            std::make_tuple(3, "single_element"), std::make_tuple(4, "repeated_max")};

const auto kTestTasksList = std::tuple_cat(ppc::util::AddFuncTask<BuzulukskiyDMaxValueMatrixElementsMPI, InType>(
                                               kTestParam, PPC_SETTINGS_buzulukskiy_d_max_value_matrix_elements),
                                           ppc::util::AddFuncTask<BuzulukskiyDMaxValueMatrixElementsSEQ, InType>(
                                               kTestParam, PPC_SETTINGS_buzulukskiy_d_max_value_matrix_elements));

const auto kGtestValues = ppc::util::ExpandToValues(kTestTasksList);
const auto kPerfTestName =
    BuzulukskiyDMaxValueMatrixElementsTests::PrintFuncTestName<BuzulukskiyDMaxValueMatrixElementsTests>;

INSTANTIATE_TEST_SUITE_P(MatrixMaxTests, BuzulukskiyDMaxValueMatrixElementsTests, kGtestValues, kPerfTestName);

}  // namespace

}  // namespace buzulukskiy_d_max_value_matrix_elements
