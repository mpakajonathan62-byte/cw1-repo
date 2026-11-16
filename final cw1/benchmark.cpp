#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <vector>
#include "Sorting.h"

struct BenchRecord { int key = 0; };
auto byKey = [](const BenchRecord& a, const BenchRecord& b) { return a.key < b.key; };

int main() {
    const std::vector<std::size_t> sizes{1000, 2000, 4000, 8000, 16000, 32000, 64000};
    std::mt19937 generator(20261116); // fixed seed makes the experiment reproducible
    std::uniform_int_distribution<int> distribution(0, 1000000000);
    std::ofstream csv("benchmark_results.csv");
    if (!csv) { std::cerr << "Cannot create benchmark_results.csv\n"; return 1; }
    csv << "n,insertion_average_ms,merge_average_ms,insertion_doubling_ratio,merge_doubling_ratio\n";

    double previousInsertion = 0.0, previousMerge = 0.0;
    for (std::size_t n : sizes) {
        double insertionTotal = 0.0, mergeTotal = 0.0;
        for (int repetition = 0; repetition < 3; ++repetition) {
            std::vector<BenchRecord> original(n);
            for (auto& record : original) record.key = distribution(generator);

            std::vector<BenchRecord> insertionData = original;
            auto start = std::chrono::steady_clock::now();
            insertionSort(insertionData, byKey);
            auto finish = std::chrono::steady_clock::now();
            insertionTotal += std::chrono::duration<double, std::milli>(finish - start).count();

            std::vector<BenchRecord> mergeData = original;
            start = std::chrono::steady_clock::now();
            mergeSort(mergeData, byKey);
            finish = std::chrono::steady_clock::now();
            mergeTotal += std::chrono::duration<double, std::milli>(finish - start).count();

            // Prevent a compiler from treating the sorted result as unused.
            if (!insertionData.empty() && !mergeData.empty() &&
                insertionData.front().key != mergeData.front().key) {
                std::cerr << "Sort results disagree.\n";
                return 2;
            }
        }
        const double insertionAverage = insertionTotal / 3.0;
        const double mergeAverage = mergeTotal / 3.0;
        const double insertionRatio = previousInsertion > 0 ? insertionAverage / previousInsertion : 0.0;
        const double mergeRatio = previousMerge > 0 ? mergeAverage / previousMerge : 0.0;
        csv << n << ',' << insertionAverage << ',' << mergeAverage << ','
            << insertionRatio << ',' << mergeRatio << '\n';
        std::cout << "n=" << n << " insertion=" << insertionAverage
                  << " ms, merge=" << mergeAverage << " ms\n";
        previousInsertion = insertionAverage;
        previousMerge = mergeAverage;
    }
    std::cout << "Saved averages and doubling ratios to benchmark_results.csv\n";
}
