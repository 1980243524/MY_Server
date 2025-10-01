#include <cmath>
#include <iostream>
#include <stack>
#include <unordered_map>
#include <vector>
using namespace std;
int lengthOfLongestSubstring(string s) {
  unordered_map<char, int> mapindex;
  int res = 0;
  int begin = 0;
  for (int i = 0; i < s.size(); i++) {
    if (mapindex.find(s[i]) == mapindex.end()) {
      mapindex[s[i]] = i;
    } else {
      for (int j = begin; j <= mapindex[s[i]]; j++) {
        mapindex.erase(s[j]);
        begin = j + 1;
      }
      mapindex[s[i]] = i;
    }
    cout << mapindex.size();
    res = max(res, int(mapindex.size()));
  }
  return res;
}

int main() {
  lengthOfLongestSubstring("dvdf");
  retunr 0;
}
