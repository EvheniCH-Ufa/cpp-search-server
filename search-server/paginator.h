#pragma once

#include <iterator>
#include <vector>

#include "document.cpp"
#include "read_input_functions.cpp"

template <typename Iterator>
class IteratorRange
{
public:
    IteratorRange(const Iterator begin, const Iterator end)
        : begin_(begin), end_(end), size_(end - begin) {
    }

    auto begin() const
    {
        return begin_;
    }

    auto end() const
    {
        return end_;
    }

    size_t size() const
    {
        return size_;
    }

private:
    Iterator begin_;
    Iterator end_;
    size_t  size_;
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& output, const IteratorRange<Iterator>& page)
{
    for (auto iter_doc = page.begin(); iter_doc != page.end(); ++iter_doc)
    {
        output << *iter_doc;     // Выводим содержимое объекта rec в output 
    }
    return output;     // Оператор должен вернуть ссылку на переданный поток вывода
}


//=====================================================================================================
template <typename Iterator>
class Paginator
{
public:
    Paginator(const Iterator begin, const Iterator end, size_t page_size)
    {
        auto curr_page_begin = begin;
        page_count_ = 0;
        size_t count_elements = distance(curr_page_begin, end);


        while (count_elements > 0)
        {
            auto page_begin = curr_page_begin;
            if (count_elements > page_size)
            {
                curr_page_begin += page_size;
            }
            else
            {
                curr_page_begin += count_elements;
            }
            auto page_end = curr_page_begin;

            IteratorRange page{ page_begin, page_end };
            pages_.push_back(page);
            ++page_count_;

            count_elements = distance(curr_page_begin, end);
        };
    }

    auto begin() const
    {
        return pages_.begin();
    }

    auto end() const
    {
        return pages_.end();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
    int page_count_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
