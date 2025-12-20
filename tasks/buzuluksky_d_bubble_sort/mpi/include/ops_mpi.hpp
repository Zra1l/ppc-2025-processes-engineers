#pragma once

#include <vector>

#include "buzuluksky_d_bubble_sort/common/include/common.hpp"
#include "mpi.h"
#include "task/include/task.hpp"

namespace buzuluksky_d_bubble_sort {

class BuzulukskyDBubbleSortMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
    return ppc::task::TypeOfTask::kMPI;
  }
  explicit BuzulukskyDBubbleSortMPI(const InType &in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

}  // namespace buzuluksky_d_bubble_sort
