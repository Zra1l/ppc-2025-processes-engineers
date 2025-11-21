# <Task Full Name>

- Student: <Бузулукский Данила Сергеевич>, group <3823Б1ПР5>
- Technology: <SEQ | MPI>
- Variant: <1>

## 1. Introduction
МОТИВАЦИЯ -> Ускорение поиска максимально элемента в матрицах большого размера,за счёт распределения нагрузки по нескольким процесам
ПРОБЛЕМА  -> Матрицы большого размера могут очень долго обрабатываться
РЕЗУЛЬТАТ -> Ускорение производительности.

## 2. Problem Statement
Formal task definition -> для матрицы размером A[a1,b1] найти максимальный элемент
input/output format    -> на вход подаётся Matrix(тоесть данные матрицы), на выход подаётся 1 int значение, которое является максимумом в матрице
constraints            -> a1,b1 > 0, matrix.size() = a1 * b1.

## 3. Baseline Algorithm (Sequential)
Describe the base algorithm with enough detail to reproduce.

```cpp
int max_value = matrix[0];
for (size_t i = 1; i < matrix.size(); i++) {
    max_value = std::max(max_value, matrix[i]);
  }
  GetOutput() = max_value;
```

## 4. Parallelization Scheme
data distribution:
Блочное распределение по строкам
Балансировка нагрузки при неравномерном распределении
, communication pattern/topology, rank roles.


## 5. Implementation Details
- Code structure (files, key classes/functions)
- Important assumptions and corner cases
- Memory usage considerations

## 6. Experimental Setup
- Hardware/OS: CPU model, cores/threads, RAM, OS version
- Toolchain: compiler, version, build type (Release/RelWithDebInfo)
- Environment: PPC_NUM_THREADS / PPC_NUM_PROC, other relevant vars
- Data: how test data is generated or sourced (relative paths)

## 7. Results and Discussion

### 7.1 Correctness
Briefly explain how correctness was verified (reference results, invariants, unit tests).

### 7.2 Performance
Present time, speedup and efficiency. Example table:

| Mode        | Count | Time, s | Speedup | Efficiency |
|-------------|-------|---------|---------|------------|
| seq         | 1     | 1.234   | 1.00    | N/A        |
| omp         | 2     | 0.700   | 1.76    | 88.0%      |
| omp         | 4     | 0.390   | 3.16    | 79.0%      |

Optionally add plots (use relative paths), and discuss bottlenecks and scalability limits.

## 8. Conclusions
Summarize findings and limitations.

## 9. References
1. <Article/Book/Doc URL>
2. <Another source>

## Appendix (Optional)
```cpp
// Short, readable code excerpts if needed