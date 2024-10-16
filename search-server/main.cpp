// p03_05_06_ObrabotkaOshibokVPoiskovojSisteme.cpp

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

bool IsSpecialSymbol(const char c)
{
    const int index = c;
    if (index >= 0 && index <= 31)
    {
        return true;
    }
    return false;
}

bool InTextLastSymbolIsMinus(const string& text)
{
    size_t text_len = text.size();
    if (text_len > 0 && text[text_len - 1] == '-')
    {
        return true;
    }
    return false;
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

struct Document {
    Document() = default;

    Document(int id, double relevance, int rating)
        : id(id)
        , relevance(relevance)
        , rating(rating) {
    }

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

template <typename StringContainer>
set<string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
    set<string> non_empty_strings;
    for (const string& str : strings) {
        if (!str.empty()) {
            non_empty_strings.insert(str);
        }
    }
    return non_empty_strings;
}

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

class SearchServer {
public:
//    -Конструкторы класса SearchServer должны выбрасывать исключение invalid_argument,
//      если любое из переданных стоп - слов содержит недопустимые символы, то есть символы с кодами от 0 до 31.

    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
        if (TextsHavesSpecialSymbol(stop_words_))
        {
            throw invalid_argument("stop_words_ haves special symbols");
        }
    }

    explicit SearchServer(const string& stop_words_text)
        : SearchServer(
            SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
    {
    }

/*
- Метод AddDocument больше не должен использовать возврат значения типа bool для сообщения об успехе или ошибке.
  Вместо этого он должен выбрасывать исключение invalid_argument в следующих ситуациях:
  - Попытка добавить документ с отрицательным id;
  - Попытка добавить документ c id ранее добавленного документа;
  - Наличие недопустимых символов (с кодами от 0 до 31) в тексте добавляемого документа.*/
    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings)
    {
        // Попытка добавить документ:
        // с отрицательным id
        if (document_id < 0) 
        {
            throw invalid_argument("id is \"-\"");
        }

        //     с сущест id          
        if (documents_.count(document_id) > 0) 
        {
            throw invalid_argument("id is exist");
        }

        //  текст со спецсимволами
        if (TextHaveSpecialSymbol(document))
        {
            throw invalid_argument("text have special symbols");
        }

        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
        document_ids_.push_back(document_id);
    }

/*  -Методы FindTopDocuments вместо возврата optional<vector<Document>> должны возвращать vector<Document> и
        выбрасывать исключение invalid_argument в следующих ситуациях :
        -В словах поискового запроса есть недопустимые символы с кодами от 0 до 31;
        -Наличие более чем одного минуса перед словами, которых не должно быть в искомых документах, например, пушистый --кот.В середине слов минусы разрешаются, например: иван - чай.
        - Отсутствие текста после символа «минус» в поисковом запросе : пушистый - .*/

    template <typename DocumentPredicate>
    vector<Document> FindTopDocuments(const string& raw_query, DocumentPredicate document_predicate) const
    {
        const Query query = ParseQuery(raw_query);

        auto result = FindAllDocuments(query, document_predicate);
        sort(result.begin(), result.end(), [](const Document& lhs, const Document& rhs) {
            if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
                return lhs.rating > rhs.rating;
            }
            else {
                return lhs.relevance > rhs.relevance;
            }
            });
        if (result.size() > MAX_RESULT_DOCUMENT_COUNT) {
            result.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return result;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const
    {
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating)
            {
                return document_status == status;
            });
    }

    vector<Document> FindTopDocuments(const string& raw_query) const
    {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    int GetDocumentCount() const {
        return documents_.size();
    }
    
/*- Метод MatchDocument должен возвращать tuple<vector<string>, DocumentStatus>, выбрасывая исключение invalid_argument
    в тех же ситуациях, что и метод FindDocument.
    - В словах поискового запроса есть недопустимые символы с кодами от 0 до 31;
    - Наличие более чем одного минуса перед словами, которых не должно быть в искомых документах, например, пушистый --кот. В середине слов минусы разрешаются, например: иван-чай.
    - Отсутствие текста после символа «минус» в поисковом запросе: пушистый -. */

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const
    {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if (word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return make_tuple(matched_words, documents_.at(document_id).status );
    }

/*    -Метод GetDocumentId должен выбрасывать исключение out_of_range, если индекс переданного документа выходит за пределы
        допустимого диапазона[0; количество документов). */
    int GetDocumentId(int document_number)
    {
        if ((document_number < 0) || (static_cast<int>(document_number + 1) > static_cast<int>(document_ids_.size())))
        {
            throw out_of_range("document_number < 0 or > document_ids_.size()");
        }
        return document_ids_[document_number];
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
    const set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, DocumentData> documents_;
    vector<int> document_ids_{};


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

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = 0;

// предлагаю пока оставить так. Мы пока итераторы не проходили (я могу, конечно:   rating_sum = accumulate(ratings.begin(), ratings.end(), 0);)
//  на этапе ревю первой работы ревьювер сказал что не надо итератор, а абычный цикл  (Федоров или Федор - не помню).
        for (const int rating : ratings) {
            rating_sum += rating;  
        }          
       
        return rating_sum / static_cast<int>(ratings.size());
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        
        if (TextHaveSpecialSymbol(text))
        {
            throw invalid_argument("Text Have Special Symbol");
        }
        
        // Word shouldn't be empty
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
            
            if (text.size() == 0)
            {
                throw invalid_argument("In Text Last Symbol Is Minus");
            }
            else
            {
                if (text[0] == ' ')
                {
                    throw invalid_argument("In Text after Minus next - Spase");
                }
            	
                if (text[0] == '-')
                {
                    throw invalid_argument("Text Have Two Minuses");
                }
			}
        }
        return { text, is_minus, IsStopWord(text) };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                }
                else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    template <typename DocumentPredicate>
    vector<Document> FindAllDocuments(const Query& query,
        DocumentPredicate document_predicate) const {
        map<int, double> document_to_relevance;
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto& [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto& [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto& [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                { document_id, relevance, documents_.at(document_id).rating });
        }
        return matched_documents;
    }
    
    bool TextHaveSpecialSymbol(const string& text) const
    {
    for (const char c : text) {
        if (IsSpecialSymbol(c))
        {
            return true;
        }
    }
    
    return false;
    }
    
	template <typename StringContainer>
	bool TextsHavesSpecialSymbol(const StringContainer& strings) {
    for (const string& str : strings) {
        if (TextHaveSpecialSymbol(str))
        {
            return true;
        }
    }
    return false;
	}
};

//Пример использования класса поисковой системы с обновлённым интерфейсом :
void PrintDocument(const Document& document) {
    cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s << endl;
}

int main() {
    SearchServer search_server("и в на"s);

    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    cout << "ACTUAL by default:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
        PrintDocument(document);
    }

    cout << "BANNED:"s << endl;
    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
        PrintDocument(document);
    }

    cout << "Even ids:"s << endl;
    for (const Document& document :
        search_server.FindTopDocuments("пушистый ухоженный кот"s,
            [](int document_id, DocumentStatus status, int rating) {
                return document_id % 2 == 0;
            }))  //
    {
        PrintDocument(document);
    }
}
