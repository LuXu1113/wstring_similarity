#include <math.h>
#include <utility>
#include <unordered_map>
#include <iostream>
#include <wchar.h>
#include "string_distance.h"

using std::cout;
using std::max;
using std::pair;
using std::make_pair;
using std::unordered_map;

StringDistance::StringDistance()
    : m_mem_buff(NULL),
      m_mem_size(0) {
        pthread_mutex_init(&m_mutex, 0);
}

StringDistance::~StringDistance() {
    pthread_mutex_destroy(&m_mutex);

    if (NULL != m_mem_buff) {
        free(m_mem_buff);
        m_mem_size = 0;
    }
}

int StringDistance::ensure_mem(size_t bytes) {
    int ret = 0;

    do {
        if (m_mem_size >= bytes) {
            ret = 0;
            break;
        }

        if (bytes > 256 * 1024 * 1024) {
            ret = ENOMEM;
            break;
        }

        if (NULL != m_mem_buff) {
            free(m_mem_buff);

            m_mem_buff = NULL;
            m_mem_size = 0;
        }

        for (m_mem_size = 512; m_mem_size < bytes; m_mem_size *= 2) {}

        m_mem_buff = (char *)malloc(m_mem_size);
        if (NULL == m_mem_buff) {
            m_mem_size = 0;
            ret = ENOMEM;
            break;
        }
    } while (0);

    return ret;
}

static int build_dict(wstring &query, wstring &subject, unordered_map<wchar_t, pair<int, int> > &dict) {
    int ret = 0;
    int l1 = subject.length();
    int l2 = query.length();

    for (int i = 0; i < l1; ++i) {
        if (dict.find(subject[i]) != dict.end()) {
            ++dict[subject[i]].first;
        } else {
            dict.insert(unordered_map<wchar_t, pair<int, int> >::value_type(subject[i], make_pair(1, 0)));
        }
    }

    for (int i = 0; i < l2; ++i) {
        if (dict.find(query[i]) != dict.end()) {
            ++dict[query[i]].second;
        } else {
            dict.insert(unordered_map<wchar_t, pair<int, int> >::value_type(query[i], make_pair(0, 1)));
        }
    }

    return ret;
}

float StringDistance::cosine_similarity(wstring &query, wstring &subject) {
    float ret = 0.0;

    unordered_map<wchar_t, pair<int, int> > dict;
    build_dict(query, subject, dict);

    float inner_prod  = 0.0;
    float length_q    = 0.0;
    float length_s    = 0.0;
    for (auto iter = dict.begin(); iter != dict.end(); ++iter) {
        inner_prod += iter->second.first * iter->second.second;
        length_s   += iter->second.first * iter->second.first;
        length_q   += iter->second.second * iter->second.second;
    }

    if (dict.size() > 0) {
        if (length_q < 1e-6 || length_s < 1e-6) {
            ret = 0.0;
        } else {
            ret = inner_prod / sqrt(length_q) / sqrt(length_s);
        }
    } else {
        ret = 1.0;
    }

    return ret;
}

float StringDistance::dice_similarity(wstring &query, wstring &subject) {
    float ret = 0.0;

    unordered_map<wchar_t, pair<int, int> > dict;
    build_dict(query, subject, dict);

    int intersection = 0;
    int size_q = 0;
    int size_s = 0;

    for (auto iter = dict.begin(); iter != dict.end(); ++iter) {
        if (iter->second.first > 0 && iter->second.second > 0) {
            ++intersection;
        }
        if (iter->second.first > 0) {
            ++size_s;
        }
        if (iter->second.second > 0) {
            ++size_q;
        }
    }

    if (dict.size() > 0) {
        ret = 2.0 * intersection / (size_s + size_q);
    } else {
        ret = 1.0;
    }

    return ret;
}

float StringDistance::jaccard_similarity(wstring &query, wstring &subject) {
    float ret = 0.0;

    unordered_map<wchar_t, pair<int, int> > dict;
    build_dict(query, subject, dict);

    int intersection = 0;

    for (auto iter = dict.begin(); iter != dict.end(); ++iter) {
        if (iter->second.first > 0 && iter->second.second > 0) {
            ++intersection;
        }
    }

    if (dict.size() > 0) {
        ret = (float)intersection / (float)dict.size();
    } else {
        ret = 1.0;
    }

    return ret;
}

float StringDistance::euclidean_distance(wstring &query, wstring &subject) {
    float ret = 0.0;

    unordered_map<wchar_t, pair<int, int> > dict;
    build_dict(query, subject, dict);

    for (auto iter = dict.begin(); iter != dict.end(); ++iter) {
        float d = iter->second.first - iter->second.second;
        ret += d * d;
    }

    ret = sqrt(ret);

    return ret;
}

const float REMOVE_COST = 2.0;
const float MODIFY_COST = 5.0;

static inline float chr_dist(const wchar_t x, const wchar_t y) {
    float ret = 0.0;

    if (x == y) {
        ret = 0.0;
    } else if (x == '\0' || y == '\0') {
        ret = REMOVE_COST;
    } else {
        ret = MODIFY_COST;
    }

    return ret;
}

float StringDistance::levenshtein_distance(wstring &query, wstring &subject) {
    float ret = 0.0;
    int l1 = subject.length();
    int l2 = query.length();

    pthread_mutex_lock(&m_mutex);
    ensure_mem(sizeof(float) * 2 * l2);

    float *dp = (float *)m_mem_buff;
    int curr = 0;
    int prev = 1;

    for (int i = 0; i < l1; ++i) {
        curr = 1 - curr;
        prev = 1 - prev;

        for (int j = 0; j < l2; ++j) {
            float dist_exchange = chr_dist(subject[i], query[j]) + ((i > 0 && j > 0) ? dp[prev * l2 + j - 1] : max(i, j) * REMOVE_COST);
            float dist_insert   = chr_dist('\0', query[j])       + ((j > 0)          ? dp[curr * l2 + j - 1] : (i + 1) * REMOVE_COST);
            float dist_remove   = chr_dist(subject[i], '\0')     + ((i > 0)          ? dp[prev * l2 + j] : (j + 1) * REMOVE_COST);

            int index = curr * l2 + j;
            dp[index] = (dist_insert < dist_exchange) ? dist_insert : dist_exchange;
            dp[index] = (dist_remove < dp[index])     ? dist_remove : dp[index];
        }
    }

    ret = dp[curr * l2 + l2 - 1];
    pthread_mutex_unlock(&m_mutex);

    return ret;
}

