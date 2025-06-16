#ifndef PMERGEME_HPP
#define PMERGEME_HPP

#include <vector>
#include <deque>

class PmergeMe {
public:
    PmergeMe();
    ~PmergeMe();

    bool parseInput(int argc, char** argv);
    void printBefore() const;
    void printAfter() const;
    void sortVector();
    void sortDeque();
    void printTiming() const;

private:
    std::vector<int> _vectorData;
    std::deque<int> _dequeData;

    std::vector<int> _vectorSorted;
    std::deque<int> _dequeSorted;

    double _vectorDurationUs;
    double _dequeDurationUs;

    bool _error;

    // Ford-Johnson algorithm implementation (vector)
    void fordJohnsonSortVector(const std::vector<int>& input, std::vector<int>& output);
    // Ford-Johnson algorithm implementation (deque)
    void fordJohnsonSortDeque(const std::deque<int>& input, std::deque<int>& output);

    // Helpers
    static void insertionSortVector(std::vector<int>& v, int start, int end);
    static void insertionSortDeque(std::deque<int>& d, int start, int end);
};

#endif

