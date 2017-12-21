#include <pthread.h>
#include <string>

using std::wstring;

class StringDistance {
    public:
        StringDistance();
        ~StringDistance();

        float cosine_similarity(wstring &query, wstring &subject);
        float dice_similarity(wstring &query, wstring &subject);
        float jaccard_similarity(wstring &query, wstring &subject);
        float euclidean_distance(wstring &query, wstring &subject);
        float levenshtein_distance(wstring &query, wstring &subject);

    private:
        int ensure_mem(size_t bytes);

        pthread_mutex_t m_mutex;
        char  *m_mem_buff;
        size_t m_mem_size;
};

