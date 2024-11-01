#pragma once

#include <deque>
#include <string>
#include <vector>
#include "search_server.h"
#include "document.h"


class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) : server_(search_server)
    {
        // напишите реализацию
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        
        std::vector<Document> result = server_.FindTopDocuments(raw_query, document_predicate);
        requests_.push_back({ raw_query, result });
        if (requests_.size() > min_in_day_)
        {
            requests_.pop_front();
        }
        return result;
    }

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        // определите, что должно быть в структуре
        std::string raw_query;
        std::vector<Document> documents_;
    };

    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    // возможно, здесь вам понадобится что-то ещё
    const SearchServer& server_;
};