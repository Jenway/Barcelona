#include "PmergeMe.hpp"
#include <iostream>
#include <ctime>
#include <sstream>
#include <limits>
#include <algorithm>

// ---------------------
// Constructor & Destructor
PmergeMe::PmergeMe() : _vectorDurationUs(0), _dequeDurationUs(0), _error(false) {}
PmergeMe::~PmergeMe() {}

#include <cstdlib>  // for strtoul
#include <cerrno>   // for errno
#include <climits>  // for ULONG_MAX

static unsigned long stoul_cpp98(const std::string& str) {
    if (str.empty()) {
        throw std::invalid_argument("stoul_cpp98: empty string");
    }

    char* endptr = NULL;
    errno = 0; // Clear errno before calling strtoul
    unsigned long val = strtoul(str.c_str(), &endptr, 10);

    // Check for conversion errors
    if (errno == ERANGE && val == ULONG_MAX) {
        throw std::out_of_range("stoul_cpp98: out of range (overflow)");
    }
    if (errno != 0 && val == 0) {
        throw std::invalid_argument("stoul_cpp98: invalid argument");
    }
    if (*endptr != '\0') {
        throw std::invalid_argument("stoul_cpp98: invalid characters in input");
    }

    return val;
}
// ---------------------
// Parse command line and validate input
bool PmergeMe::parseInput(int argc, char** argv) {
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg.empty()) {
            _error = true;
            return false;
        }
        // Check if all digits
        for (size_t j = 0; j < arg.size(); j++) {
            if (!isdigit(arg[j])) {
                _error = true;
                return false;
            }
        }
        // Convert to int and check positive
        try {
            unsigned long val = stoul_cpp98(arg);
            if (val == 0 || val > static_cast<unsigned long>(std::numeric_limits<int>::max())) {

                _error = true;
                return false;
            }
            int v = static_cast<int>(val);
            _vectorData.push_back(v);
            _dequeData.push_back(v);
        } catch (...) {
            _error = true;
            return false;
        }
    }
    return !_error;
}

// ---------------------
// Printing before sorting
void PmergeMe::printBefore() const {
    std::cout << "Before:";
    for (size_t i = 0; i < _vectorData.size(); i++)
        std::cout << " " << _vectorData[i];
    std::cout << std::endl;
}

// ---------------------
// Printing after sorting
void PmergeMe::printAfter() const {
    std::cout << "After:";
    for (size_t i = 0; i < _vectorSorted.size(); i++)
        std::cout << " " << _vectorSorted[i];
    std::cout << std::endl;
}

// ---------------------
// Timing and sorting with vector
void PmergeMe::sortVector() {
    clock_t start = clock();
    fordJohnsonSortVector(_vectorData, _vectorSorted);
    clock_t end = clock();
    _vectorDurationUs = (end - start) * 1000000.0 / CLOCKS_PER_SEC;
}

void PmergeMe::sortDeque() {
    clock_t start = clock();
    fordJohnsonSortDeque(_dequeData, _dequeSorted);
    clock_t end = clock();
    _dequeDurationUs = (end - start) * 1000000.0 / CLOCKS_PER_SEC;
}

// ---------------------
// Print timing info
void PmergeMe::printTiming() const {
    size_t n = _vectorData.size();
    std::cout << "Time to process a range of " << n << " elements with std::vector : " << _vectorDurationUs << " us\n";
    std::cout << "Time to process a range of " << n << " elements with std::deque : " << _dequeDurationUs << " us\n";
}

// ---------------------
// Simple insertion sort for small ranges (vector)
void PmergeMe::insertionSortVector(std::vector<int>& v, int start, int end) {
    for (int i = start + 1; i <= end; i++) {
        int key = v[i];
        int j = i - 1;
        while (j >= start && v[j] > key) {
            v[j + 1] = v[j];
            j--;
        }
        v[j + 1] = key;
    }
}

// ---------------------
// Simple insertion sort for small ranges (deque)
void PmergeMe::insertionSortDeque(std::deque<int>& d, int start, int end) {
    for (int i = start + 1; i <= end; i++) {
        int key = d[i];
        int j = i - 1;
        while (j >= start && d[j] > key) {
            d[j + 1] = d[j];
            j--;
        }
        d[j + 1] = key;
    }
}

// ---------------------
// Ford-Johnson (Merge-Insertion) sort for vector
void PmergeMe::fordJohnsonSortVector(const std::vector<int>& input, std::vector<int>& output) {
    // For small inputs use insertion sort directly
    if (input.size() <= 1) {
        output = input;
        return;
    }
    if (input.size() <= 10) {
        output = input;
        insertionSortVector(output, 0, (int)output.size() - 1);
        return;
    }

    // Implement simplified version of Ford-Johnson for large input:
    // - split input into pairs
    // - sort each pair, separate larger and smaller
    // - recursively sort larger elements
    // - insert smaller elements into sorted larger with binary search

    // 1) Create vector of pairs (smaller, larger)
    std::vector<int> larger;
    std::vector<int> smaller;
    for (size_t i = 0; i + 1 < input.size(); i += 2) {
        if (input[i] < input[i + 1]) {
            smaller.push_back(input[i]);
            larger.push_back(input[i + 1]);
        } else {
            smaller.push_back(input[i + 1]);
            larger.push_back(input[i]);
        }
    }
    // If odd number of elements, last element is treated as larger element
    if (input.size() % 2 != 0)
        larger.push_back(input.back());

    // 2) Recursively sort the larger elements
    std::vector<int> sortedLarger;
    fordJohnsonSortVector(larger, sortedLarger);

    // 3) Insert smaller elements one by one into sortedLarger with binary search
    output = sortedLarger;
    for (size_t i = 0; i < smaller.size(); i++) {
        int val = smaller[i];
        // binary search for insertion position
        int left = 0, right = (int)output.size();
        while (left < right) {
            int mid = (left + right) / 2;
            if (output[mid] < val)
                left = mid + 1;
            else
                right = mid;
        }
        output.insert(output.begin() + left, val);
    }
}

// ---------------------
// Ford-Johnson sort for deque (same logic but on deque container)
void PmergeMe::fordJohnsonSortDeque(const std::deque<int>& input, std::deque<int>& output) {
    if (input.size() <= 1) {
        output = input;
        return;
    }
    if (input.size() <= 10) {
        output = input;
        insertionSortDeque(output, 0, (int)output.size() - 1);
        return;
    }

    std::deque<int> larger;
    std::deque<int> smaller;
    for (size_t i = 0; i + 1 < input.size(); i += 2) {
        if (input[i] < input[i + 1]) {
            smaller.push_back(input[i]);
            larger.push_back(input[i + 1]);
        } else {
            smaller.push_back(input[i + 1]);
            larger.push_back(input[i]);
        }
    }
    if (input.size() % 2 != 0)
        larger.push_back(input.back());

    std::deque<int> sortedLarger;
    fordJohnsonSortDeque(larger, sortedLarger);

    output = sortedLarger;
    for (size_t i = 0; i < smaller.size(); i++) {
        int val = smaller[i];
        int left = 0, right = (int)output.size();
        while (left < right) {
            int mid = (left + right) / 2;
            if (output[mid] < val)
                left = mid + 1;
            else
                right = mid;
        }
        output.insert(output.begin() + left, val);
    }
}

