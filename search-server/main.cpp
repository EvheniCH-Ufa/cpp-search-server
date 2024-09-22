#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cmath>


using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine()
{
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber()
{
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text)
{
    vector<string> words;
    string word;
    for (const char c : text)
    {
        if (c == ' ')
        {
            if (!word.empty())
            {
                words.push_back(word);
                word.clear();
            }
        }
        else
        {
            word += c;
        }
    }
    if (!word.empty())
    {
        words.push_back(word);
    }

    return words;
}


struct Document
{
    int id;
    double relevance;
};

struct Query
{
    set<string> plus_words_;
    set<string> minus_words;
};



class SearchServer
{
public:
    void SetStopWords(const string& text)
    {
        for (const string& word : SplitIntoWords(text))
        {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document)
    {
        vector<string> document_word_no_stop = SplitIntoWordsNoStop(document);

        double word_frequency = 1.0 / document_word_no_stop.size(); // Замечание №1(+)

        for (const string& word : document_word_no_stop) 
        {
            key_words_documents_TF_[word][document_id] += word_frequency;  // то есть если это слово будет дважды, то оно прибавится
        }
        ++documents_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        const Query query = ParseQuery(raw_query);  // popravil
        auto matched_documents = FindAllDocuments(query);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs)
            {
                return lhs.relevance > rhs.relevance;
            });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
        {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int documents_count_ = 0;

    // key_word     id     TF
    map<string, map<int, double>> key_words_documents_TF_;

    set<string> stop_words_;


    double CalculateIDF(const string& word) const
    {
        int count_docs_id_tf = key_words_documents_TF_.at(word).size(); // мощность этого словаря (количество)
        double idf;

        // тут проверка, если кол-во доков == мощности словаря, то есть, слово есть во всех доках., то IDF=0
        if (count_docs_id_tf == documents_count_)   
        {
            idf = 0;   // если слово есть во всех доках, то грош ему цена (= 0)
        }
        else
        {
            idf = 1.0 * log(1.0 * documents_count_ / count_docs_id_tf);
        }
        return idf;
    }

    bool IsStopWord(const string& word) const
    {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const
    {
        vector<string> words;
        for (const string& word : SplitIntoWords(text))
        {
            if (!IsStopWord(word))
            {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const
    {
        Query query;
        for (const string& word : SplitIntoWordsNoStop(text))
        {
            if (word[0] != '-')
            {
                query.plus_words_.insert(word);
            }
            else
            {
                query.minus_words.insert(word.substr(1, word.size() - 1)); // удаляем первый символ и записываем в банк минус-слов);
            }
        }                                      
        return query;
    }

    vector<Document> FindAllDocuments(const Query& query) const
    {
        //  id   relev
        map<int, double> findet_docs;
        vector <string> minus_words;

        if (!query.plus_words_.empty()) 
        {
            for (const string& query_plus_word : query.plus_words_)
            {
                double idf = CalculateIDF(query_plus_word); // Замечание №4(2)(+)

                auto docs_id_tf = key_words_documents_TF_.at(query_plus_word);  // записали в переменную словарь с id и tf документов по этому слову

                for (const auto& doc_id_tf : docs_id_tf)                  // обход по map_у найденных документов (мап) по ключу ИД 
                {
                    findet_docs[doc_id_tf.first] += (1.0 * doc_id_tf.second * idf);  // и увеличиваем релевантность уже на эту TF-IDF - типа суммируем
                }  // так стало, конечно, значительно проще 
            }

            for (string query_minus_word : query.minus_words)
            {
                //               тут тоже убрал итератор
                auto docs_id_tf = key_words_documents_TF_.at(query_minus_word);  // записали в переменную словарь с id и tf документов по этому minus-слову
                for (const auto& doc_id_tf : docs_id_tf)                         // в найденных документах (мап) по ключу ИД обход
                {
                    findet_docs.erase(doc_id_tf.first);                          // и удаляем из найденных доков эти документы 
                }
            }
        }
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
    for (int document_id = 0; document_id < document_count; ++document_id)
    {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main()
{
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& document : search_server.FindTopDocuments(query))
    {
        cout << "{ document_id = "s << document.id << ", " << "relevance = "s << document.relevance << " }"s << endl;
    }

    // system("pause");
}
