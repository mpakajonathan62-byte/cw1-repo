#include <iostream>
#include <string>
#include <vector>
#include "Doublylinkedlist.h"
#include "Sorting.h"

struct TestRecord {
    int id = 0;
    int key = 0;
    std::string label;
    bool isValid() const { return id > 0; }
};

auto byKey = [](const TestRecord& a, const TestRecord& b) { return a.key < b.key; };
auto keyOf = [](const TestRecord& value) { return value.key; };

int main() {
    int passed = 0, failed = 0;
    auto check = [&](const std::string& name, bool condition) {
        std::cout << (condition ? "PASS: " : "FAIL: ") << name << '\n';
        condition ? ++passed : ++failed;
    };

    // A1 boundary cases
    DoublyLinkedList<TestRecord> list;
    check("empty structure", list.empty() && list.size() == 0);
    check("remove from empty structure", !list.remove(99));
    check("insert one record", list.pushBack({1, 10, "one"}) && list.size() == 1);
    check("duplicate ID rejected", !list.pushBack({1, 20, "duplicate"}) && list.size() == 1);
    check("insert at front", list.pushFront({2, 5, "front"}) && list.find(2) != nullptr);
    check("remove first record", list.remove(2) && list.find(1) != nullptr);
    check("insert at back", list.pushBack({3, 30, "back"}));
    check("remove last record", list.remove(3) && list.size() == 1);
    check("remove remaining record", list.remove(1) && list.empty());

    // A2 sorting and binary-search cases
    std::vector<TestRecord> empty;
    insertionSort(empty, byKey);
    mergeSort(empty, byKey);
    check("sort empty vector", empty.empty());

    std::vector<TestRecord> one{{1, 7, "only"}};
    mergeSort(one, byKey);
    check("sort one record", one.size() == 1 && one[0].key == 7);

    std::vector<TestRecord> sorted{{1, 1, "a"}, {2, 2, "b"}, {3, 3, "c"}};
    insertionSort(sorted, byKey);
    check("already sorted input", sorted[0].key == 1 && sorted[2].key == 3);

    std::vector<TestRecord> insertionReversed{{1, 3, "c"}, {2, 2, "b"}, {3, 1, "a"}};
    insertionSort(insertionReversed, byKey);
    check("reversed input sorted by insertion sort", insertionReversed[0].key == 1 && insertionReversed[2].key == 3);

    std::vector<TestRecord> reversed{{1, 3, "c"}, {2, 2, "b"}, {3, 1, "a"}};
    mergeSort(reversed, byKey);
    check("reversed input sorted by merge sort", reversed[0].key == 1 && reversed[1].key == 2 && reversed[2].key == 3);

    std::vector<TestRecord> duplicates{{1, 2, "first"}, {2, 1, "middle"}, {3, 2, "second"}};
    mergeSort(duplicates, byKey);
    check("duplicate keys retain original order (stable merge sort)",
          duplicates[1].label == "first" && duplicates[2].label == "second");

    std::vector<TestRecord> searchValues{{1, 10, "ten"}, {2, 20, "twenty"}, {3, 30, "thirty"}};
    const auto found = binarySearch(searchValues, 20, keyOf, std::less<int>{});
    const auto absent = binarySearch(searchValues, 25, keyOf, std::less<int>{});
    check("binary search finds existing key", found >= 0 && searchValues[static_cast<std::size_t>(found)].key == 20);
    check("binary search reports absent key", absent == -1);
    check("binary search on empty vector", binarySearch(empty, 4, keyOf, std::less<int>{}) == -1);

    std::cout << "\nTest summary: " << passed << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}
