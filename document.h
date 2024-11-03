#pragma once

#include <iostream>

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating);
    
    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

std::ostream& operator<<(std::ostream& output, const Document document)
{
    using namespace std::literals;
    output << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;

    return output;     // Оператор должен вернуть ссылку на переданный поток вывода
}