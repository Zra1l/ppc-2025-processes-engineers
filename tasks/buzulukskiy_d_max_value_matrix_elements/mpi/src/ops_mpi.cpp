#include "buzulukskiy_d_max_value_matrix_elements/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <numeric>
#include <vector>

#include "buzulukskiy_d_max_value_matrix_elements/common/include/common.hpp"
#include "util/include/util.hpp"

namespace buzulukskiy_d_max_value_matrix_elements {

BuzulukskiyDMaxValueMatrixElementsMPI::BuzulukskiyDMaxValueMatrixElementsMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
  GetOutput() = 0;
}

bool BuzulukskiyDMaxValueMatrixElementsMPI::ValidationImpl() {
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

bool BuzulukskiyDMaxValueMatrixElementsMPI::PreProcessingImpl() {
  return true;
}

bool BuzulukskiyDMaxValueMatrixElementsMPI::RunImpl() {
  const Matrix& inputdata = GetInput();
  const int rows = inputdata.rows;
  const int columns = inputdata.columns;
  const std::vector<int>& matrix = inputdata.data;
  int rank,size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  const int rows_at_one_proces = rows/size;
  const int remaining_rows = rows%size;
  std::vector<int> how_many_to_one_proces(size, rows_at_one_proces*columns);
  std::vector<int> offset(size, 0);


  

  return true;
}

bool BuzulukskiyDMaxValueMatrixElementsMPI::PostProcessingImpl() {
  return true;
}

}  // namespace buzulukskiy_d_max_value_matrix_elements
