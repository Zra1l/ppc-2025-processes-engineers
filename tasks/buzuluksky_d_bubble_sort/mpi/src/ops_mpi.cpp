#include "buzuluksky_d_bubble_sort/mpi/include/ops_mpi.hpp"

#include <algorithm>
#include <cstddef>
#include <vector>

#include "buzuluksky_d_bubble_sort/common/include/common.hpp"
#include "mpi.h"

namespace buzuluksky_d_bubble_sort {

namespace {

void LocalOddEvenSort(std::vector<int> &array) {
  if (array.empty()) {
    return;
  }

  const size_t n = array.size();
  bool sorted = false;

  for (size_t pass = 0; pass < n && !sorted; ++pass) {
    sorted = true;

    for (size_t i = 0; i + 1 < n; i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }

    for (size_t i = 1; i + 1 < n; i += 2) {
      if (array[i] > array[i + 1]) {
        std::swap(array[i], array[i + 1]);
        sorted = false;
      }
    }

    if (sorted) {
      break;
    }
  }
}

int GetCommunicationPartner(int rank, int phase, int proc_count) {
  if (phase % 2 == 0) {
    if (rank % 2 == 0 && rank + 1 < proc_count) {
      return rank + 1;
    }
    if (rank % 2 != 0 && rank - 1 >= 0) {
      return rank - 1;
    }
  } else {
    if (rank % 2 == 0 && rank - 1 >= 0) {
      return rank - 1;
    }
    if (rank % 2 != 0 && rank + 1 < proc_count) {
      return rank + 1;
    }
  }
  return -1;
}

std::vector<int> ExchangeWithPartner(int partner, const std::vector<int> &local) {
  int my_size = static_cast<int>(local.size());
  int partner_size = 0;

  MPI_Sendrecv(&my_size, 1, MPI_INT, partner, 0, &partner_size, 1, MPI_INT, partner, 0, MPI_COMM_WORLD,
               MPI_STATUS_IGNORE);

  std::vector<int> partner_data(partner_size);
  MPI_Sendrecv(local.data(), my_size, MPI_INT, partner, 1, partner_data.data(), partner_size, MPI_INT, partner, 1,
               MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  return partner_data;
}

std::vector<int> MergeAndSelect(int rank, int partner, const std::vector<int> &local, const std::vector<int> &remote) {
  std::vector<int> merged(local.size() + remote.size());
  std::merge(local.begin(), local.end(), remote.begin(), remote.end(), merged.begin());

  std::vector<int> result(local.size());
  if (rank < partner) {
    std::copy_n(merged.begin(), local.size(), result.begin());
  } else {
    std::copy_n(merged.end() - static_cast<int>(local.size()), local.size(), result.begin());
  }

  return result;
}

void PerformOddEvenPhases(int rank, int proc_count, std::vector<int> &local) {
  for (int phase = 0; phase < proc_count; ++phase) {
    int partner = GetCommunicationPartner(rank, phase, proc_count);

    if (partner != -1) {
      std::vector<int> partner_data = ExchangeWithPartner(partner, local);
      local = MergeAndSelect(rank, partner, local, partner_data);
    }

    MPI_Barrier(MPI_COMM_WORLD);
  }
}

}  // namespace

BuzulukskyDBubbleSortMPI::BuzulukskyDBubbleSortMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool BuzulukskyDBubbleSortMPI::ValidationImpl() {
  return true;
}

bool BuzulukskyDBubbleSortMPI::PreProcessingImpl() {
  return true;
}

bool BuzulukskyDBubbleSortMPI::RunImpl() {
  int rank = 0;
  int proc_count = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &proc_count);

  const auto &input = GetInput();
  int total_size = static_cast<int>(input.size());

  if (total_size == 0) {
    GetOutput() = std::vector<int>();
    return true;
  }

  if (total_size == 1) {
    GetOutput() = input;
    return true;
  }

  int working_procs = std::min(proc_count, total_size);

  if (rank >= working_procs) {
    if (rank == 0) {
      GetOutput() = std::vector<int>();
    } else {
      GetOutput() = std::vector<int>();
    }
    return true;
  }

  std::vector<int> counts(working_procs, 0);
  std::vector<int> offsets(working_procs, 0);

  if (rank == 0) {
    int base = total_size / working_procs;
    int extra = total_size % working_procs;
    int offset = 0;

    for (int i = 0; i < working_procs; ++i) {
      counts[i] = base + (i < extra ? 1 : 0);
      offsets[i] = offset;
      offset += counts[i];
    }
  }

  MPI_Bcast(counts.data(), working_procs, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(offsets.data(), working_procs, MPI_INT, 0, MPI_COMM_WORLD);

  std::vector<int> local_data(counts[rank]);

  if (total_size > 0) {
    MPI_Scatterv(rank == 0 ? input.data() : nullptr, counts.data(), offsets.data(), MPI_INT, local_data.data(),
                 counts[rank], MPI_INT, 0, MPI_COMM_WORLD);
  }

  LocalOddEvenSort(local_data);

  PerformOddEvenPhases(rank, working_procs, local_data);

  std::vector<int> result;
  if (rank == 0) {
    result.resize(total_size);
  }

  if (total_size > 0) {
    MPI_Gatherv(local_data.data(), static_cast<int>(local_data.size()), MPI_INT, rank == 0 ? result.data() : nullptr,
                counts.data(), offsets.data(), MPI_INT, 0, MPI_COMM_WORLD);
  }

  if (rank == 0) {
    GetOutput() = std::move(result);
  } else {
    GetOutput() = std::vector<int>();
  }
  return true;
}

bool BuzulukskyDBubbleSortMPI::PostProcessingImpl() {
  return true;
}

}  // namespace buzuluksky_d_bubble_sort
