#include "request_queue.h"


std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentStatus status) {
    // напишите реализацию
    std::vector<Document> result = server_.FindTopDocuments(raw_query, status);
    requests_.push_back({ raw_query, result });
    if (requests_.size() > min_in_day_)
    {
        requests_.pop_front();
    }

    return result;
}

std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query) {
    // напишите реализацию
    std::vector<Document> result = server_.FindTopDocuments(raw_query);
    requests_.push_back({ raw_query, result });
    if (requests_.size() > min_in_day_)
    {
        requests_.pop_front();
    }
    return result;
}

int RequestQueue::GetNoResultRequests() const {
    // напишите реализацию
    int empty_requests_count = std::count_if(requests_.begin(), requests_.end(), [](const QueryResult& query_result) {
        return query_result.documents_.empty(); });
    return empty_requests_count;
}
