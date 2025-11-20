#include "buzulukskiy_d_max_value_matrix_elements/seq/include/ops_seq.hpp"

#include <numeric>
#include <vector>

#include "buzulukskiy_d_max_value_matrix_elements/common/include/common.hpp"
#include "util/include/util.hpp"

namespace buzulukskiy_d_max_value_matrix_elements {

BuzulukskiyDMaxValueMatrixElementsSEQ::BuzulukskiyDMaxValueMatrixElementsSEQ(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BuzulukskiyDMaxValueMatrixElementsSEQ::ValidationImpl() {
  const Matrix& inputdata = GetInput();
  const int rows = inputdata.rows;
  const int columns = inputdata.columns;
  const std::vector<int>& matrix = inputdata.data;

  if(matrix.empty() || rows <= 0 || columns <= 0 || matrix.size() != (size_t)rows*columns)
  {
    return false;
  }
  return true;
}

bool BuzulukskiyDMaxValueMatrixElementsSEQ::PreProcessingImpl() {
  return true;
}

bool BuzulukskiyDMaxValueMatrixElementsSEQ::RunImpl() {
  const Matrix& inputdata = GetInput();
  const std::vector<int>& matrix = inputdata.data;

  int max_value = matrix[0];

  for(size_t i = 0;i < matrix.size();i++)
  {
    max_value = std::max(max_value, matrix[i]);
  }
  GetOutput() = max_value;
  return true;
}

bool BuzulukskiyDMaxValueMatrixElementsSEQ::PostProcessingImpl() {
  return true;
}

}  // namespace buzulukskiy_d_max_value_matrix_elements
