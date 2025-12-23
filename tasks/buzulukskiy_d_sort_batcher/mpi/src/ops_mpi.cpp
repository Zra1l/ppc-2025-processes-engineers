#include "buzulukskiy_d_sort_batcher/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <array>
#include <climits>
#include <cstddef>
#include <iterator>
#include <tuple>
#include <utility>
#include <vector>

namespace buzulukskiy_d_sort_batcher {

namespace {
constexpr int kRadixBase = 10;
constexpr int kMaxIterations = 100;

void RadixSortUnsigned(std::vector<int> &arr) {
  if (arr.empty()) {
    return;
  }

  const int max_val = *std::ranges::max_element(arr);
  std::vector<int> output(arr.size());

  for (int exp = 1; max_val / exp > 0; exp *= kRadixBase) {
    std::array<int, kRadixBase> count = {0};

    for (const int value : arr) {
      ++count[(value / exp) % kRadixBase];
    }

    for (int i = 1; i < kRadixBase; ++i) {
      count[i] += count[i - 1];
    }

    for (std::size_t i = arr.size(); i-- > 0;) {
      const int digit = (arr[i] / exp) % kRadixBase;
      output[--count[digit]] = arr[i];
    }

    arr.swap(output);
  }
}

void RadixSortLSD(std::vector<int> &data) {
  if (data.empty()) {
    return;
  }

  std::vector<int> positives;
  std::vector<int> negatives;

  for (const int value : data) {
    if (value < 0) {
      negatives.push_back(-value);
    } else {
      positives.push_back(value);
    }
  }

  if (!positives.empty()) {
    RadixSortUnsigned(positives);
  }

  if (!negatives.empty()) {
    RadixSortUnsigned(negatives);
    std::ranges::reverse(negatives);
    for (int &value : negatives) {
      value = -value;
    }
  }

  data.clear();
  data.insert(data.end(), negatives.begin(), negatives.end());
  data.insert(data.end(), positives.begin(), positives.end());
}

void ExchangeAndMerge(std::vector<int> &local, int partner, const std::vector<int> &counts, int rank) {
  if (partner < 0 || std::cmp_greater_equal(partner, static_cast<int>(counts.size()))) {
    return;
  }

  const auto remote_size = static_cast<std::size_t>(counts[static_cast<std::size_t>(partner)]);
  std::vector<int> remote(remote_size);

  MPI_Sendrecv(local.data(), static_cast<int>(local.size()), MPI_INT, partner, 0, remote.data(),
               static_cast<int>(remote.size()), MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  std::vector<int> combined;
  combined.reserve(local.size() + remote.size());
  std::ranges::merge(local, remote, std::back_inserter(combined));

  const auto my_count = static_cast<std::size_t>(counts[static_cast<std::size_t>(rank)]);
  if (my_count > combined.size()) {
    return;
  }

  if (rank < partner) {
    local.assign(combined.begin(), combined.begin() + static_cast<std::ptrdiff_t>(my_count));
  } else {
    local.assign(combined.end() - static_cast<std::ptrdiff_t>(my_count), combined.end());
  }
}

void BatcherStabilizationPhase(std::vector<int> &local, int rank, int size, const std::vector<int> &counts) {
  const int stabilization_steps = std::min(size, kMaxIterations);

  for (int step = 0; step < stabilization_steps; ++step) {
    int partner = 0;
    if (step % 2 == 0) {
      partner = (rank % 2 == 0) ? rank + 1 : rank - 1;
    } else {
      partner = (rank % 2 != 0) ? rank + 1 : rank - 1;
    }

    if (partner >= 0 && partner < size) {
      ExchangeAndMerge(local, partner, counts, rank);
    }
    MPI_Barrier(MPI_COMM_WORLD);
  }
}

void BatcherNetworkPhase(std::vector<int> &local, int rank, int size, const std::vector<int> &counts) {
  for (int phase = 1; phase < size; phase <<= 1) {
    for (int k = phase; k > 0; k >>= 1) {
      for (int j = k % phase; j + k < size; j += 2 * k) {
        for (int i = 0; i < k && i + j + k < size; ++i) {
          const int r1 = i + j;
          const int r2 = i + j + k;

          if ((r1 / (phase * 2)) == (r2 / (phase * 2))) {
            if (rank == r1 || rank == r2) {
              const int partner = (rank == r1) ? r2 : r1;
              ExchangeAndMerge(local, partner, counts, rank);
            }
          }
        }
      }
      MPI_Barrier(MPI_COMM_WORLD);
    }
  }
}

std::tuple<std::vector<int>, std::vector<int>, std::size_t> CalculateDistribution(int n, int size, int rank) {
  std::vector<int> counts(static_cast<std::size_t>(size), 0);
  std::vector<int> displs(static_cast<std::size_t>(size), 0);

  const int base = n / size;
  const int rem = n % size;
  int offset = 0;

  for (int i = 0; i < size; ++i) {
    counts[static_cast<std::size_t>(i)] = base + (i < rem ? 1 : 0);
    displs[static_cast<std::size_t>(i)] = offset;
    offset += counts[static_cast<std::size_t>(i)];
  }

  const auto local_size = static_cast<std::size_t>(counts[static_cast<std::size_t>(rank)]);
  return {counts, displs, local_size};
}

}  // namespace

BuzulukskiyDSortBatcherMPI::BuzulukskiyDSortBatcherMPI(const InType &in) {
  SetTypeOfTask(GetStaticTypeOfTask());
  GetInput() = in;
}

bool BuzulukskiyDSortBatcherMPI::ValidationImpl() {
  return true;
}

bool BuzulukskiyDSortBatcherMPI::PreProcessingImpl() {
  return true;
}

bool BuzulukskiyDSortBatcherMPI::PostProcessingImpl() {
  return true;
}

bool BuzulukskiyDSortBatcherMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int n = 0;
  if (rank == 0) {
    n = static_cast<int>(GetInput().size());
  }
  MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

  if (n == 0) {
    if (rank == 0) {
      GetOutput() = InType();
    }
    return true;
  }

  auto [counts, displs, local_size] = CalculateDistribution(n, size, rank);

  std::vector<int> local(local_size);
  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, counts.data(), displs.data(), MPI_INT, local.data(),
               static_cast<int>(local_size), MPI_INT, 0, MPI_COMM_WORLD);

  if (!local.empty()) {
    RadixSortLSD(local);
  }

  BatcherNetworkPhase(local, rank, size, counts);
  BatcherStabilizationPhase(local, rank, size, counts);

  std::vector<int> result(static_cast<std::size_t>(n));
  MPI_Allgatherv(local.data(), static_cast<int>(local.size()), MPI_INT, result.data(), counts.data(), displs.data(),
                 MPI_INT, MPI_COMM_WORLD);

  GetOutput() = std::move(result);
  return true;
}

}  // namespace buzulukskiy_d_sort_batcher
