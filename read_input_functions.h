#pragma once

#include "document.h"
#include <iostream>


std::ostream& operator<<(std::ostream& output, const Document document)
{
    using namespace std::literals;
    output << "{ document_id = "s << document.id << ", relevance = "s << document.relevance << ", rating = "s << document.rating << " }"s;

    return output;     // Оператор должен вернуть ссылку на переданный поток вывода
}

