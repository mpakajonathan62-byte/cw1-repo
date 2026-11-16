#ifndef DOUBLY_LINKED_LIST_H
#define DOUBLY_LINKED_LIST_H

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <new>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

// Generic doubly linked list used as the A1 storage structure.
template <typename T>
class DoublyLinkedList {
private:
    struct Node {
        T data;
        Node* prev;
        Node* next;
        explicit Node(const T& value) : data(value), prev(nullptr), next(nullptr) {}
    };

    Node* head;
    Node* tail;
    std::size_t count;

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head, other.head);
        std::swap(tail, other.tail);
        std::swap(count, other.count);
    }

    bool contains(int id) const {
        for (Node* current = head; current != nullptr; current = current->next)
            if (current->data.id == id) return true;
        return false;
    }

public:
    DoublyLinkedList() : head(nullptr), tail(nullptr), count(0) {}

    DoublyLinkedList(const DoublyLinkedList& other) : head(nullptr), tail(nullptr), count(0) {
        try {
            for (Node* current = other.head; current != nullptr; current = current->next) {
                if (!pushBack(current->data)) throw std::bad_alloc();
            }
        } catch (...) {
            clear();
            throw;
        }
    }

    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head(other.head), tail(other.tail), count(other.count) {
        other.head = nullptr;
        other.tail = nullptr;
        other.count = 0;
    }

    DoublyLinkedList& operator=(DoublyLinkedList other) noexcept {
        swap(other);
        return *this;
    }

    ~DoublyLinkedList() { clear(); }

    bool pushBack(const T& value) {
        if (!value.isValid() || contains(value.id)) return false;
        Node* node = nullptr;
        try { node = new Node(value); }
        catch (const std::bad_alloc&) { return false; }
        node->prev = tail;
        if (tail) tail->next = node;
        else head = node;
        tail = node;
        ++count;
        return true;
    }

    bool pushFront(const T& value) {
        if (!value.isValid() || contains(value.id)) return false;
        Node* node = nullptr;
        try { node = new Node(value); }
        catch (const std::bad_alloc&) { return false; }
        node->next = head;
        if (head) head->prev = node;
        else tail = node;
        head = node;
        ++count;
        return true;
    }

    T* find(int id) {
        for (Node* current = head; current != nullptr; current = current->next)
            if (current->data.id == id) return &current->data;
        return nullptr;
    }

    const T* find(int id) const {
        for (Node* current = head; current != nullptr; current = current->next)
            if (current->data.id == id) return &current->data;
        return nullptr;
    }

    bool remove(int id) {
        Node* current = head;
        while (current) {
            if (current->data.id == id) {
                if (current->prev) current->prev->next = current->next;
                else head = current->next;
                if (current->next) current->next->prev = current->prev;
                else tail = current->prev;
                delete current;
                --count;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool update(int id, const T& replacement) {
        if (!replacement.isValid() || replacement.id != id) return false;
        T* record = find(id);
        if (!record) return false;
        try { *record = replacement; }
        catch (const std::bad_alloc&) { return false; }
        return true;
    }

    void clear() noexcept {
        Node* current = head;
        while (current) {
            Node* next = current->next;
            delete current;
            current = next;
        }
        head = nullptr;
        tail = nullptr;
        count = 0;
    }

    std::size_t size() const { return count; }
    bool empty() const { return count == 0; }

    template <typename Function>
    void forEach(Function function) const {
        for (Node* current = head; current != nullptr; current = current->next)
            function(current->data);
    }

    std::vector<T> toVector() const {
        std::vector<T> result;
        result.reserve(count);
        forEach([&result](const T& value) { result.push_back(value); });
        return result;
    }

    // Replace contents only if every record is valid and IDs are unique.
    // Sorting the auxiliary ID vector makes duplicate validation O(n log n),
    // rather than repeatedly calling pushBack (which would make this O(n^2)).
    bool replaceWith(const std::vector<T>& values) {
        std::vector<int> ids;
        ids.reserve(values.size());
        for (const T& value : values) {
            if (!value.isValid()) return false;
            ids.push_back(value.id);
        }
        std::sort(ids.begin(), ids.end());
        for (std::size_t i = 1; i < ids.size(); ++i)
            if (ids[i] == ids[i - 1]) return false;

        DoublyLinkedList<T> temporary;
        try {
            for (const T& value : values) {
                Node* node = new Node(value);
                node->prev = temporary.tail;
                if (temporary.tail) temporary.tail->next = node;
                else temporary.head = node;
                temporary.tail = node;
                ++temporary.count;
            }
        } catch (const std::bad_alloc&) {
            return false; // temporary destructor releases any nodes already built
        }
        swap(temporary);
        return true;
    }

    bool loadFromFile(const std::string& filename, std::size_t& loaded,
                      std::size_t& rejected) {
        std::ifstream file(filename);
        loaded = 0;
        rejected = 0;
        if (!file.is_open()) return false;
        DoublyLinkedList<T> temporary;
        std::string line;
        while (std::getline(file, line)) {
            // Windows text files commonly use CRLF line endings. std::getline
            // removes '\n' but can leave the '\r', which would make the
            // final date field fail YYYY-MM-DD validation.
            if (!line.empty() && line.back() == '\r') line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            std::istringstream parser(line);
            T record{};
            if (parser >> record) {
                if (temporary.pushBack(record)) ++loaded;
                else ++rejected;
            } else {
                ++rejected;
            }
        }
        if (file.bad()) return false;
        swap(temporary);
        return true;
    }

    bool loadFromFile(const std::string& filename, std::size_t& loaded) {
        std::size_t rejected = 0;
        return loadFromFile(filename, loaded, rejected);
    }

    bool saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) return false;
        for (Node* current = head; current != nullptr; current = current->next) {
            file << current->data << '\n';
            if (!file) return false;
        }
        return file.good();
    }

    void display(std::ostream& out) const {
        forEach([&out](const T& value) { out << value << '\n'; });
    }
};

#endif
