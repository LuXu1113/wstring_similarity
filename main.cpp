#include <string>
#include <vector>
#include <iostream>
#include "string_distance.h"

using std::wstring;
using std::vector;
using std::wcout;
using std::endl;

int main () {
    std::locale::global(std::locale(""));
    StringDistance worker;

    wstring query = L"秀春刀";
    wstring candidates[10] = {L"海云台", L"秀春刀·修罗战场", L"我家乐翻天", L"清醒梦2之疯狂快递", L"表情奇幻冒险", L"喜欢你", L"傲娇与偏见", L"英伦对决", L"教数学的体育老师", L"长城"};

    wcout << L"cosine_similarity: " << endl;
    for (int i = 0; i < 10; ++i) {
        float sim = worker.cosine_similarity(query, candidates[i]);
        wcout << L"[" << query << L" : " << candidates[i] << L"]: " << sim << endl;
    }

    wcout << L"dice_similarity: " << endl;
    for (int i = 0; i < 10; ++i) {
        float sim = worker.dice_similarity(query, candidates[i]);
        wcout << L"[" << query << L" : " << candidates[i] << L"]: " << sim << endl;
    }

    wcout << L"jaccard_similarity: " << endl;
    for (int i = 0; i < 10; ++i) {
        float sim = worker.jaccard_similarity(query, candidates[i]);
        wcout << L"[" << query << L" : " << candidates[i] << L"]: " << sim << endl;
    }

    wcout << L"euclidean_distance: " << endl;
    for (int i = 0; i < 10; ++i) {
        float sim = worker.euclidean_distance(query, candidates[i]);
        wcout << L"[" << query << L" : " << candidates[i] << L"]: " << sim << endl;
    }

    wcout << L"levenshtein_distance: " << endl;
    for (int i = 0; i < 10; ++i) {
        float sim = worker.levenshtein_distance(query, candidates[i]);
        wcout << L"[" << query << L" : " << candidates[i] << L"]: " << sim << endl;
    }

    return 0;
}

