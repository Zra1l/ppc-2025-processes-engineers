#include "buzulukskiy_d_sort_batcher/mpi/include/ops_mpi.hpp"

#include <mpi.h>

#include <algorithm>
#include <climits>
#include <cstddef>
#include <iostream>
#include <vector>

namespace buzulukskiy_d_sort_batcher {

namespace {
constexpr int kRadixBase = 10;
constexpr int kMaxIterations = 100;

void RadixSortLSD(std::vector<int> &data) {
  if (data.empty()) {
    return;
  }

  const long long minVal = static_cast<long long>(*std::min_element(data.begin(), data.end()));

  if (minVal < 0) {
    for (auto &value : data) {
      const long long shiftedValue = static_cast<long long>(value) - minVal;
      if (shiftedValue > static_cast<long long>(INT_MAX)) {
        value = INT_MAX;
      } else {
        value = static_cast<int>(shiftedValue);
      }
    }
  }

  const int maxVal = *std::max_element(data.begin(), data.end());
  std::vector<int> output(data.size());

  for (int exp = 1; maxVal / exp > 0; exp *= kRadixBase) {
    int count[kRadixBase] = {0};

    for (const int value : data) {
      ++count[(value / exp) % kRadixBase];
    }

    for (int i = 1; i < kRadixBase; ++i) {
      count[i] += count[i - 1];
    }

    for (std::size_t idx = data.size(); idx-- > 0;) {
      const int digit = (data[idx] / exp) % kRadixBase;
      output[--count[digit]] = data[idx];
    }

    data.swap(output);
  }

  if (minVal < 0) {
    for (auto &value : data) {
      const long long originalValue = static_cast<long long>(value) + minVal;
      if (originalValue < static_cast<long long>(INT_MIN)) {
        value = INT_MIN;
      } else if (originalValue > static_cast<long long>(INT_MAX)) {
        value = INT_MAX;
      } else {
        value = static_cast<int>(originalValue);
      }
    }
  }
}

void ExchangeAndMerge(std::vector<int> &local, int partner, const std::vector<int> &counts, int rank) {
  if (partner < 0 || partner >= static_cast<int>(counts.size())) {
    return;
  }

  const std::size_t remoteSize = static_cast<std::size_t>(counts[static_cast<std::size_t>(partner)]);
  std::vector<int> remote(remoteSize);

  MPI_Sendrecv(local.data(), static_cast<int>(local.size()), MPI_INT, partner, 0, remote.data(),
               static_cast<int>(remote.size()), MPI_INT, partner, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  std::vector<int> combined;
  combined.reserve(local.size() + remote.size());
  std::merge(local.begin(), local.end(), remote.begin(), remote.end(), std::back_inserter(combined));

  const std::size_t myCount = static_cast<std::size_t>(counts[static_cast<std::size_t>(rank)]);
  if (myCount > combined.size()) {
    return;
  }

  if (rank < partner) {
    local.assign(combined.begin(), combined.begin() + static_cast<std::ptrdiff_t>(myCount));
  } else {
    local.assign(combined.end() - static_cast<std::ptrdiff_t>(myCount), combined.end());
  }
}

void BatcherStabilizationPhase(std::vector<int> &local, int rank, int size, const std::vector<int> &counts) {
  const int stabilizationSteps = std::min(size, kMaxIterations);

  for (int step = 0; step < stabilizationSteps; ++step) {
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
  for (int p = 1; p < size; p <<= 1) {
    for (int k = p; k > 0; k >>= 1) {
      for (int j = k % p; j + k < size; j += 2 * k) {
        for (int i = 0; i < k && i + j + k < size; ++i) {
          const int r1 = i + j;
          const int r2 = i + j + k;

          if ((r1 / (p * 2)) == (r2 / (p * 2))) {
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

  const std::size_t localSize = static_cast<std::size_t>(counts[static_cast<std::size_t>(rank)]);
  return {counts, displs, localSize};
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

  auto [counts, displs, localSize] = CalculateDistribution(n, size, rank);

  std::vector<int> local(localSize);
  MPI_Scatterv(rank == 0 ? GetInput().data() : nullptr, counts.data(), displs.data(), MPI_INT, local.data(),
               static_cast<int>(localSize), MPI_INT, 0, MPI_COMM_WORLD);

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
