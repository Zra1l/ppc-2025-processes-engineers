- Student: <Бузулукский Данила Сергеевич>, group <3823Б1ПР5>
- Technology: <SEQ | MPI>
- Variant: <19>

## 1. Introduction
МОТИВАЦИЯ -> Ускорение сортировки массивов большого размера за счёт распределения нагрузки по нескольким процессам.
ПРОБЛЕМА  ->  Алгоритмы сортировки с квадратичной сложностью неэффективны для больших объемов данных. Необходим алгоритм, сочетающий высокую производительность с возможностью параллельного выполнения.
РЕЗУЛЬТАТ -> Реализация поразрядной сортировки с четно-нечетным слиянием Бэтчера, обеспечивающая значительное ускорение обработки за счет параллельной реализации на MPI.

## 2. Problem Statement
Formal task definition ->  Отсортировать массив целых чисел по возрастанию с использованием поразрядной сортировки и сети четно-нечетного слияния Бэтчера.
input/output format    -> На вход подаётся std::vector<int>, на выходе - отсортированный std::vector<int> той же длины
constraints            -> N >= 0, все элементы - целые числа

## 3. Baseline Algorithm (Sequential)
Describe the base algorithm with enough detail to reproduce.

```cpp
  // Алгоритм LSD Radix Sort с обработкой отрицательных чисел
void RadixSortLSD(std::vector<int>& data) {
    if (data.empty()) return;
    
    // Разделение на положительные и отрицательные числа
    std::vector<int> positives, negatives;
    for (int v : data) {
        if (v < 0) negatives.push_back(v == INT_MIN ? INT_MAX : -v);
        else positives.push_back(v);
    }
    
    // Сортировка обеих частей
    if (!positives.empty()) RadixSortUnsigned(positives);
    if (!negatives.empty()) {
        RadixSortUnsigned(negatives);
        std::reverse(negatives.begin(), negatives.end());
        for (int& v : negatives) v = (v == INT_MAX) ? INT_MIN : -v;
    }
    
    // Объединение результатов
    data.clear();
    if (!negatives.empty()) data.insert(data.end(), negatives.begin(), negatives.end());
    if (!positives.empty()) data.insert(data.end(), positives.begin(), positives.end());
}

//Вспомогательная функция сортировки неотрицательных чисел:
void RadixSortUnsigned(std::vector<int>& arr) {
    if (arr.empty()) return;
    
    int maxVal = *std::max_element(arr.begin(), arr.end());
    std::vector<int> output(arr.size());
    
    for (int exp = 1; maxVal / exp > 0; exp *= 10) {
        int count[10] = {0};
        for (int v : arr) count[(v / exp) % 10]++;
        for (int i = 1; i < 10; i++) count[i] += count[i-1];
        for (std::size_t i = arr.size(); i-- > 0;) {
            int digit = (arr[i] / exp) % 10;
            output[--count[digit]] = arr[i];
        }
        arr.swap(output);
    }
}

//Четно-нечетное слияние Бэтчера:
std::vector<int> BatcherOddEvenMerge(const std::vector<int>& a, const std::vector<int>& b) {
    std::vector<int> result;
    result.reserve(a.size() + b.size());
    std::merge(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(result));
    return result;
}
```
Про алгоритм:
 Алгоритм стабильно работает с любым количеством процессов(а не как обычный с 2 4 8 16, теперь и с 6 и тд),
 Алгоритм был реализован так, что отрицательные и положительные числа обрабатываются отдельно
 Также алгоритм стабилен, благодаря обратному проходу при распределении
 Алгоритм не завсит от начального расположения значений(тоесть с ревёрснутыми числами он тоже отлично справляется)
 Способен работать с отрицательными числами

## 4. Parallelization Scheme
data distribution:
Блочное распределение массива между процессами с балансировкой нагрузки
Каждый процесс получает примерно n/size элементов
, communication pattern:
```cpp
MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);     // Передача размера
MPI_Scatterv(...);                                // Распределение данных
BatcherNetworkPhase(...);                         // Сеть Бэтчера
BatcherStabilizationPhase(...);                   // Фаза стабилизации
MPI_Allgatherv(...);                              // Сбор результатов
```
 rank roles:
    Rank 0: инициализация, распределение и сбор результатов
    Отсальные процессы: локальная сортировка и участие в сети Бэтчира


## 5. Implementation Details
- Code structure (files, key classes/functions)
common - общие структуры данных
mpi - паралельная реализация mpi
seq - последовательная
test - тесты функцианальности и производительности

```cpp
// Базовые типы данных
using InType = std::vector<int>;
using OutType = std::vector<int>;
using TestType = std::tuple<InType, std::string>;
using BaseTask = ppc::task::Task<InType, OutType>;

// MPI реализация
class BuzulukskiyDSortBatcherMPI : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
      return ppc::task::TypeOfTask::kMPI;
  }

  explicit BuzulukskiyDSortBatcherMPI(const InType& in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};

// SEQ реализация  
class BuzulukskiyDSortBatcherSEQ : public BaseTask {
 public:
  static constexpr ppc::task::TypeOfTask GetStaticTypeOfTask() {
      return ppc::task::TypeOfTask::kSEQ;
  }

  explicit BuzulukskiyDSortBatcherSEQ(const InType& in);

 private:
  bool ValidationImpl() override;
  bool PreProcessingImpl() override;
  bool RunImpl() override;
  bool PostProcessingImpl() override;
};
```
- Important assumptions and corner cases
 1- масив может быть пустым
 2- элементы могут быть отрицательными
 3- дубликаты могут быть
 4- лишние процессы получают нулевой размер локальных данных
- Memory usage considerations
 1- каждый процесс хранит свою часть масива
 2- избегается фрагментация памяти через предварительное резервирование
 3- mpi буферы для комуникации
 4- нет утечек памяти благодаря использованию RAII (std::vector)


## 6. Experimental Setup
- Процессор: ryzen 5 5600x
- Количество ядер: 6
- Количество потоков: 12
- ОЗУ: 32 Гб
- ОС: Windows 10
- Архитектура: x64

- Язык программирования: C++
- Библиотека для параллельного программирования: MPI
- Компилятор MSCV
- Тип сборки: Release 

## 7. Results and Discussion

### 7.1 Correctness
Все тесты на коретность были успешно пройдены, тестов было 10

### 7.2 Performance
Таблица с данными при тестировании перформенса:

| Mode        | processes | AvgTime(s) | Speedup | Efficiency |
|-------------|-----------|------------|---------|------------|
| seq         | 1         | 0.02500    | 1.00    | N/A        |
| mpi         | 2         | 0.01050    | 2.38    | 119%       |
| mpi         | 4         | 0.01300    | 1.92    |  48%       |
| mpi         | 6         | 0.01850    | 1.35    |  23%       |
| mpi         | 8         | 0.01950    | 1.28    |  16%       |


## 8. Conclusions
ВЫВОД :использование mpi показало свою эффективность, ускорив работу в 2 раз в максимальном варианте таким образом можно сделать вывод, что распределение нагрузки на несколько процессов является максимально эффективным метод работы с большим количеством данных(так как тут не самое большое количество данных, то далее идёт спад по скорости, но если увеличить ещё сильнее то будет заметно увелечение скорости и эффективности)
Наибольший прирост производительности наблюдается на 2 процессах
Ограничения: Не стоит делать с маленькими масивами, так как там нет выйгрыша, по времени
Следовательно использование MPI является эффективным в условиях больших масивах когда комуникация между процессами не затратит времени больше, чем сама работа с данными

## 9. References
  MICROSOFT MPI - https://learn.microsoft.com/ru-ru/message-passing-interface/microsoft-mpi
  Parallel Programming Course - https://learning-process.github.io/parallel_programming_course/ru/index.html
  Parallel Programming 2025-2026 - https://disk.yandex.ru/d/NvHFyhOJCQU65w
  stack overflow - https://stackoverflow.com/questions