#ifndef SPAN_HPP
#define SPAN_HPP

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <climits>

class Span {
public:
    Span(unsigned int N);
    Span(const Span& other);
    Span& operator=(const Span& other);
    ~Span();

    void addNumber(int number);

    template <typename InputIt>
    void addNumber(InputIt begin, InputIt end) {
        if (_numbers.size() + std::distance(begin, end) > _maxSize)
            throw std::overflow_error("Span: Not enough space for range insert");
        _numbers.insert(_numbers.end(), begin, end);
    }

    int shortestSpan() const;
    int longestSpan() const;

private:
    unsigned int _maxSize;
    std::vector<int> _numbers;
};

#endif

