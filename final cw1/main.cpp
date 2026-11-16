#include <cstddef>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>
#include "Doublylinkedlist.h"
#include "Sorting.h"

namespace utils {
bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

bool extractDateParts(const std::string& date, int& year, int& month, int& day) {
    if (date.size() != 10 || date[4] != '-' || date[7] != '-') return false;
    for (std::size_t i = 0; i < date.size(); ++i) {
        if (i == 4 || i == 7) continue;
        if (date[i] < '0' || date[i] > '9') return false;
    }
    try {
        year = std::stoi(date.substr(0, 4));
        month = std::stoi(date.substr(5, 2));
        day = std::stoi(date.substr(8, 2));
    } catch (...) { return false; }
    if (year < 1900 || year > 2200 || month < 1 || month > 12 || day < 1) return false;
    constexpr int daysInMonth[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int maximum = daysInMonth[month - 1];
    if (month == 2 && isLeapYear(year)) ++maximum;
    return day <= maximum;
}

bool checkDateFormat(const std::string& date) {
    int year = 0, month = 0, day = 0;
    return extractDateParts(date, year, month, day);
}

long computeEpochDays(const std::string& date) {
    int year = 0, month = 0, day = 0;
    if (!extractDateParts(date, year, month, day)) return -1;
    if (month <= 2) --year;
    const long era = (year >= 0 ? year : year - 399) / 400;
    const long yearOfEra = year - era * 400;
    const long dayOfYear = (153L * (month + (month > 2 ? -3 : 9)) + 2) / 5 + day - 1;
    const long dayOfEra = yearOfEra * 365 + yearOfEra / 4 - yearOfEra / 100 + dayOfYear;
    return era * 146097 + dayOfEra - 719468;
}

bool hasValidCharacters(const std::string& value) {
    bool hasContent = false;
    for (char ch : value) {
        if (ch == '|' || ch == '\n' || ch == '\r') return false;
        if (ch != ' ' && ch != '\t') hasContent = true;
    }
    return hasContent;
}
} // namespace utils

struct ReturnRecord {
    int id = 0;
    std::string memberName;
    std::string bookTitle;
    std::string dueDate;
    std::string returnDate;

    int daysLate() const {
        const long due = utils::computeEpochDays(dueDate);
        const long returned = utils::computeEpochDays(returnDate);
        if (due < 0 || returned < 0 || returned <= due) return 0;
        return static_cast<int>(returned - due);
    }
    long fineUGX() const { return static_cast<long>(daysLate()) * 500L; }
    bool isValid() const {
        return id > 0 && utils::hasValidCharacters(memberName) &&
               utils::hasValidCharacters(bookTitle) &&
               utils::checkDateFormat(dueDate) && utils::checkDateFormat(returnDate);
    }
};

std::ostream& operator<<(std::ostream& out, const ReturnRecord& record) {
    out << record.id << '|' << record.memberName << '|' << record.bookTitle << '|'
        << record.dueDate << '|' << record.returnDate;
    return out;
}

std::istream& operator>>(std::istream& in, ReturnRecord& record) {
    std::string line;
    if (!std::getline(in, line)) return in;
    std::istringstream row(line);
    std::string idText;
    ReturnRecord parsed;
    if (!std::getline(row, idText, '|') || !std::getline(row, parsed.memberName, '|') ||
        !std::getline(row, parsed.bookTitle, '|') || !std::getline(row, parsed.dueDate, '|') ||
        !std::getline(row, parsed.returnDate)) {
        in.setstate(std::ios::failbit);
        return in;
    }
    try {
        std::size_t used = 0;
        parsed.id = std::stoi(idText, &used);
        if (used != idText.size() || parsed.id <= 0) {
            in.setstate(std::ios::failbit);
            return in;
        }
    } catch (...) { in.setstate(std::ios::failbit); return in; }
    if (!parsed.isValid()) { in.setstate(std::ios::failbit); return in; }
    record = parsed;
    return in;
}

namespace io_helpers {
const char* DEFAULT_DATASET = R"DATA(# Library returns register sample data
# id|member|title|due|returned
1001|Achieng Grace|Things Fall Apart|2026-10-20|2026-10-22
1002|Okello David|Introduction to Algorithms|2026-10-18|2026-10-18
1003|Nakato Sarah|Data Structures in C++|2026-10-21|2026-10-25
1004|Mugisha Peter|The River Between|2026-10-19|2026-10-17
1005|Namukasa Ruth|Operating System Concepts|2026-10-22|2026-10-22
1006|Ssemwogerere Ivan|Clean Code|2026-10-15|2026-10-24
1007|Atim Joan|Weep Not, Child|2026-10-23|2026-10-22
1008|Kato Brian|Computer Networks|2026-10-20|2026-10-25
1009|Nabirye Esther|Database System Concepts|2026-10-24|2026-10-25
1010|Opio Samuel|The Pragmatic Programmer|2026-10-17|2026-10-25
1011|Tumusiime Alice|Discrete Mathematics|2026-10-25|2026-10-25
1012|Byaruhanga Moses|Artificial Intelligence: A Modern Approach|2026-10-16|2026-10-22
)DATA";

bool createDefaultFile(const std::string& filename) {
    { std::ifstream existing(filename); if (existing.good()) return true; }
    std::ofstream output(filename);
    if (!output.is_open()) return false;
    output << DEFAULT_DATASET;
    return output.good();
}

void printTableHeader() {
    std::cout << std::left << std::setw(7) << "ID" << std::setw(21) << "Member"
              << std::setw(32) << "Book" << std::setw(13) << "Due"
              << std::setw(13) << "Returned" << std::right << std::setw(7) << "Late"
              << std::setw(12) << "Fine UGX" << '\n';
    std::cout << std::string(105, '-') << '\n';
}
void printRecord(const ReturnRecord& record) {
    std::cout << std::left << std::setw(7) << record.id
              << std::setw(21) << record.memberName.substr(0, 20)
              << std::setw(32) << record.bookTitle.substr(0, 31)
              << std::setw(13) << record.dueDate << std::setw(13) << record.returnDate
              << std::right << std::setw(7) << record.daysLate()
              << std::setw(12) << record.fineUGX() << '\n';
}
} // namespace io_helpers

using ReturnsRegister = DoublyLinkedList<ReturnRecord>;

void clearInputLine() { std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); }
int readInteger(const std::string& prompt) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) { clearInputLine(); return value; }
        if (std::cin.eof()) return -1;
        std::cout << "Please enter a whole number.\n";
        std::cin.clear();
        clearInputLine();
    }
}
std::string readText(const std::string& prompt) {
    std::string value;
    std::cout << prompt;
    std::getline(std::cin, value);
    return value;
}
ReturnRecord readRecordFromUser(int id) {
    ReturnRecord record;
    record.id = id;
    record.memberName = readText("Member name: ");
    record.bookTitle = readText("Book title: ");
    record.dueDate = readText("Due date (YYYY-MM-DD): ");
    record.returnDate = readText("Return date (YYYY-MM-DD): ");
    return record;
}

void displayRecords(const ReturnsRegister& records) {
    if (records.empty()) { std::cout << "The returns register is empty.\n"; return; }
    io_helpers::printTableHeader();
    records.forEach([](const ReturnRecord& record) { io_helpers::printRecord(record); });
}
void displayVector(const std::vector<ReturnRecord>& records) {
    if (records.empty()) { std::cout << "The returns register is empty.\n"; return; }
    io_helpers::printTableHeader();
    for (const auto& record : records) io_helpers::printRecord(record);
}
void addRecord(ReturnsRegister& records) {
    const int id = readInteger("Enter new record ID: ");
    if (id <= 0) { std::cout << "Invalid ID.\n"; return; }
    ReturnRecord record = readRecordFromUser(id);
    if (!record.isValid()) { std::cout << "Record rejected: check the name, title and dates.\n"; return; }
    if (records.pushBack(record)) std::cout << "Record added.\n";
    else std::cout << "Record not added. The ID may already exist or memory may be unavailable.\n";
}
void searchRecord(const ReturnsRegister& records) {
    const int id = readInteger("Enter record ID to search: ");
    const ReturnRecord* record = records.find(id);
    if (!record) { std::cout << "No record found with that ID.\n"; return; }
    io_helpers::printTableHeader();
    io_helpers::printRecord(*record);
}
void updateRecord(ReturnsRegister& records) {
    const int id = readInteger("Enter ID of record to update: ");
    if (!records.find(id)) { std::cout << "No record found with that ID.\n"; return; }
    ReturnRecord replacement = readRecordFromUser(id);
    if (!replacement.isValid()) { std::cout << "Update rejected: check the name, title and dates.\n"; return; }
    std::cout << (records.update(id, replacement) ? "Record updated.\n" : "Record could not be updated.\n");
}
void deleteRecord(ReturnsRegister& records) {
    const int id = readInteger("Enter ID of record to delete: ");
    std::cout << (records.remove(id) ? "Record deleted.\n" : "No record found with that ID.\n");
}

// Scenario key: due date. Several library loans can share a due date, so a
// stable sort keeps their original register order within each due-date group.
auto compareByDueDate = [](const ReturnRecord& a, const ReturnRecord& b) { return a.dueDate < b.dueDate; };
auto dueDateOf = [](const ReturnRecord& record) -> const std::string& { return record.dueDate; };

void sortRecords(ReturnsRegister& records, bool useMergeSort) {
    std::vector<ReturnRecord> values = records.toVector();
    if (useMergeSort) {
        mergeSort(values, compareByDueDate);
        std::cout << "Records sorted by due date using stable merge sort.\n";
    } else {
        insertionSort(values, compareByDueDate);
        std::cout << "Records sorted by due date using stable insertion sort.\n";
    }
    if (!records.replaceWith(values)) {
        std::cout << "Could not replace register contents after sorting.\n";
        return;
    }
    displayRecords(records);
}
void binarySearchRecord(const ReturnsRegister& records) {
    std::vector<ReturnRecord> sorted = records.toVector();
    mergeSort(sorted, compareByDueDate); // Search key and sort key must be identical.
    const std::string date = readText("Enter due date to find (YYYY-MM-DD): ");
    if (!utils::checkDateFormat(date)) { std::cout << "Invalid date format.\n"; return; }
    const std::ptrdiff_t index = binarySearch(sorted, date, dueDateOf, std::less<std::string>{});
    if (index < 0) { std::cout << "No record found with that due date.\n"; return; }
    io_helpers::printTableHeader();
    io_helpers::printRecord(sorted[static_cast<std::size_t>(index)]);
}

int main() {
    const std::string filename = "sample_data.txt";
    if (!io_helpers::createDefaultFile(filename)) {
        std::cerr << "Could not create or access " << filename << ".\n";
        return 1;
    }
    ReturnsRegister records;
    std::size_t loaded = 0, rejected = 0;
    if (!records.loadFromFile(filename, loaded, rejected)) {
        std::cerr << "Could not load " << filename << ".\n";
        return 1;
    }
    std::cout << "Library Returns Register\n" << loaded << " valid record(s) loaded";
    if (rejected) std::cout << "; " << rejected << " invalid or duplicate line(s) skipped";
    std::cout << ".\n";

    bool running = true;
    while (running) {
        std::cout << "\n========== MENU ==========\n"
                  << "1. Display all records\n"
                  << "2. Add a record\n"
                  << "3. Search for a record (linked-list search)\n"
                  << "4. Update a record\n"
                  << "5. Delete a record\n"
                  << "6. Save records\n"
                  << "7. Exit\n"
                  << "8. Sort by due date (insertion sort)\n"
                  << "9. Sort by due date (merge sort)\n"
                  << "10. Search by due date (iterative binary search)\n";
        const int choice = readInteger("Choose an option: ");
        switch (choice) {
            case 1: displayRecords(records); break;
            case 2: addRecord(records); break;
            case 3: searchRecord(records); break;
            case 4: updateRecord(records); break;
            case 5: deleteRecord(records); break;
            case 6: std::cout << (records.saveToFile(filename) ? "Records saved to " + filename + ".\n" : "Could not save the records.\n"); break;
            case 7:
                if (records.saveToFile(filename)) std::cout << "Records saved. Goodbye.\n";
                else std::cout << "Warning: records could not be saved.\n";
                running = false;
                break;
            case 8: sortRecords(records, false); break;
            case 9: sortRecords(records, true); break;
            case 10: binarySearchRecord(records); break;
            default:
                if (choice == -1 && std::cin.eof()) { records.saveToFile(filename); running = false; }
                else std::cout << "Choose a valid menu number.\n";
                break;
        }
    }
    return 0;
}
