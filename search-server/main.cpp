#include <algorithm>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <cmath>

// Табуляция и, соответственно, скобки "плывут" после Dev-c++

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
        if (c == ' ') {
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

        double tf = 1.0 / document_word_no_stop.size();; // Замечание №1(+)
/*        if (document_word_no_stop.size() > 0)   // тут была реализована "защита от дурака", т.к априори входящий документ может полностью состоять из стоп-слов и сюда вернуться уже пустым. Но если надо - уберу
        {
            tf = 1.0 / document_word_no_stop.size();
        }*/

        for (const string& word : document_word_no_stop)  // Замечание №2(+): Честно сказать еще просто не привык писать так и забыл перед отправкой все перепроверить на КОНСТ и ССЫЛОЧНОСТЬ
        {
            key_words_documents_TF_[word][document_id] += tf;  // то есть если это слово будет дважды, то оно прибавится
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


    double CalculateIDF (const map<int, double>& docs_id_tf) const
    {
        int count_docs_id_tf = docs_id_tf.size(); // мощность этого словаря (количество)
        double idf;
        
        // тут проверка, если кол-во доков == мощности словаря, то есть слово есть во всех доках., то IDF=0
        if (count_docs_id_tf == documents_count_)   // Замечание №6(+) перенес ---------------------------------------------------------------
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
                query.minus_words.insert(word.substr(1, word.size()-1)); // удаляем первый символ и записываем в банк минус-слов);
            }
        }                                        // Замечание №3(+): готово
        return query;
    }                                            

    vector<Document> FindAllDocuments(const Query& query) const // Замечание №4(+): переписал 
    {
        //  id   relev
        map<int, double> findet_docs;
        vector <string> minus_words;

        if (!query.plus_words_.empty()) // Замечание №5(+): то есть, если плюс-слов нет, то и по минусам нечего шахаться
          { 
            for (const string& query_plus_word : query.plus_words_) 
            {
/*              Замечание №7(+): я, конечно, убрал итератор. но мне кажется что это хорошая защита от дурака, т.к.
                at() генерит исключение, если в запросе будет слово, которого нет ни в одном документе либо вручную бегать по всему МАПу
                но, видимо, мы к это вернемся в теме "Обработка исключений" try....        */
                map <int, double> docs_id_tf = key_words_documents_TF_.at(query_plus_word);  // записали в переменную словарь с id и tf документов по этому слову

                double idf = CalculateIDF(docs_id_tf); // Замечание №6(+): вынес отдельно, но с проверкой на то, что слова не входит во все доки сразу 
                          
                for (const auto& doc_id_tf : docs_id_tf)                  // обход по map_у найденных документов (мап) по ключу ИД 
                {
                    findet_docs[doc_id_tf.first] += (1.0 * doc_id_tf.second * idf);  // и увеличиваем релевантность уже на эту TF-IDF - типа суммируем
                }  // так стало, конечно, значительно проще 
            }        
            
            for (string query_minus_word : query.minus_words)
            {
//               тут тоже убрал итератор
                auto docs_id_tf = key_words_documents_TF_.at(query_minus_word);  // записали в переменную словарь с id и tf документов по этому minus-слову
                	for (const auto& doc_id_tf : docs_id_tf)                     // в найденных документах (мап) по ключу ИД обход
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
