#include <algorithm>
#include <iostream>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>


using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document
{
    int id;
    double relevance;
};


class SearchServer
{
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document)
    {
        vector<string> document_word_no_stop = SplitIntoWordsNoStop(document);

        double tf = 0.0;
        if (document_word_no_stop.size() > 0)
        {
            tf = 1.0 / document_word_no_stop.size();
        }

        for (string word : document_word_no_stop)
        {
            key_words_documents_TF_[word][document_id] += tf;  // то есть если это слово будет дважды, то оно прибавится
        }
        ++documents_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        const set<string> query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int documents_count_ = 0;

    // key_word     id     TF
    map<string, map<int, double>> key_words_documents_TF_;

    set<string> stop_words_;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    set<string> ParseQuery(const string& text) const {
        set<string> query_words;
        for (const string& word : SplitIntoWordsNoStop(text)) {
            query_words.insert(word);
        }
        return query_words;
    }

    vector<Document> FindAllDocuments(const set<string>& query_words) const
    {
        //  id   relev
        map<int, double> findet_docs;
        vector <string> minus_words;

        for (const string& query_word : query_words)
        {
            if (query_word.size() > 0)
            {
                if (query_word[0] == '-')                           // то есть, это минус-слово
                {
                    minus_words.push_back(query_word.substr(1, query_word.size()-1)); // удаляем первый символ и записываем в банк минус-слов
                }
                else
                {
                    auto iterator_current_key_word = key_words_documents_TF_.find(query_word);  // иначе ищем документы по слову запроса в банке слов 
                    if (iterator_current_key_word != key_words_documents_TF_.end())             // если нашли
                    {
                        auto& docs_id_tf = iterator_current_key_word->second;                    // записали в переменную словарь с id и tf документов по этому слову
                        int docs_count_po_query_word = docs_id_tf.size();   // мощность этого словаря (количество)    
                        
                        for (const auto& doc_id_tf : docs_id_tf)                  // обход по map_у найденных документов (мап) по ключу ИД 
                        {
                            // тут проверка, если кол-во доков == мощности словаря, то есть слово есть во всех доках., то IDF=0
                            double idf = 0;
                            if (docs_count_po_query_word != documents_count_)
                            {
                                idf = 1.0 * log(1.0 * documents_count_ / docs_count_po_query_word);
                            }
                            findet_docs[doc_id_tf.first] += (1.0 * doc_id_tf.second * idf);  // и увеличиваем релевантность уже на эту TF-IDF - типа суммируем
                        }  // блин, замудрено-то как....
                    }
                }
            }
        }

        for (string minus_word : minus_words)
        {
            auto iterator_current_minus_word = key_words_documents_TF_.find(minus_word);  // теперь ищем по минус-слову индекс в банке слов - в каких доках оно есть
            if (iterator_current_minus_word != key_words_documents_TF_.end())             // если нашли
            {
                for (const auto& doc_id_tf : iterator_current_minus_word->second)                     // в найденных документах (мап) по ключу ИД обход
                {
                    findet_docs.erase(doc_id_tf.first);                          // и удаляем из найденных доков эти документы 
                }
            }
        };

        vector<Document> matched_documents;

        for (const auto findet_doc : findet_docs)
        {
            matched_documents.push_back({ findet_doc.first, findet_doc.second });
        }

        return matched_documents;
    }       
};


SearchServer CreateSearchServer()
{
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main()
{
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& document : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document.id << ", "
            << "relevance = "s << document.relevance << " }"s << endl;
    }

    //   system("pause");
}
