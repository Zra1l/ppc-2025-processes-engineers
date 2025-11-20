#pragma once

#include <string>
#include <tuple>

#include "task/include/task.hpp"

namespace buzulukskiy_d_max_value_matrix_elements {

using InType = int;
using OutType = int;
using TestType = std::tuple<int, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

}  // namespace buzulukskiy_d_max_value_matrix_elements
