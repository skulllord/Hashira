#include <iostream>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cassert>

using namespace std;

class BigInt {
public:
    string value;

    BigInt() : value("0") {}
    BigInt(string v) : value(trim(v)) {}
    BigInt(long long v) : value(to_string(v)) {}

    static string trim(const string &s) {
        size_t i = 0;
        while (i < s.length() && s[i] == '0') i++;
        return i == s.length() ? "0" : s.substr(i);
    }

    static string addStrings(string a, string b) {
        string result = "";
        int carry = 0, sum = 0;
        int i = a.size() - 1, j = b.size() - 1;
        while (i >= 0 || j >= 0 || carry) {
            int d1 = i >= 0 ? a[i--] - '0' : 0;
            int d2 = j >= 0 ? b[j--] - '0' : 0;
            sum = d1 + d2 + carry;
            carry = sum / 10;
            result += (sum % 10 + '0');
        }
        reverse(result.begin(), result.end());
        return trim(result);
    }

    static string multiplyStrings(string num1, string num2) {
        if (num1 == "0" || num2 == "0") return "0";
        vector<int> res(num1.size() + num2.size(), 0);
        for (int i = num1.size() - 1; i >= 0; i--) {
            for (int j = num2.size() - 1; j >= 0; j--) {
                res[i + j + 1] += (num1[i] - '0') * (num2[j] - '0');
            }
        }
        for (int i = res.size() - 1; i > 0; i--) {
            res[i - 1] += res[i] / 10;
            res[i] %= 10;
        }
        string result;
        for (int r : res) {
            if (!(result.empty() && r == 0)) result += to_string(r);
        }
        return trim(result);
    }

    static string divideString(string num, int divisor) {
        string result;
        int idx = 0, temp = num[0] - '0';
        while (temp < divisor) temp = temp * 10 + (num[++idx] - '0');
        while (num.size() > idx) {
            result += (temp / divisor) + '0';
            temp = (temp % divisor) * 10 + num[++idx] - '0';
        }
        return result.empty() ? "0" : trim(result);
    }

    BigInt operator+(const BigInt &b) const {
        return BigInt(addStrings(value, b.value));
    }

    BigInt operator*(const BigInt &b) const {
        return BigInt(multiplyStrings(value, b.value));
    }

    BigInt operator/(int divisor) const {
        return BigInt(divideString(value, divisor));
    }

    
    BigInt operator-() const {
        if (value == "0") return BigInt("0");
        return BigInt("-" + value);
    }

    BigInt operator-(const BigInt &b) const {
        bool neg = false;
        string a = value, bb = b.value;
        if (a == bb) return BigInt("0");

        if (a.length() < bb.length() || (a.length() == bb.length() && a < bb)) {
            swap(a, bb);
            neg = true;
        }

        string result = "";
        int carry = 0, diff;
        int i = a.length() - 1, j = bb.length() - 1;

        while (i >= 0 || j >= 0) {
            int d1 = i >= 0 ? a[i--] - '0' : 0;
            int d2 = j >= 0 ? bb[j--] - '0' : 0;
            diff = d1 - d2 - carry;
            if (diff < 0) {
                diff += 10;
                carry = 1;
            } else {
                carry = 0;
            }
            result += (diff + '0');
        }
        while (result.size() > 1 && result.back() == '0') result.pop_back();
        reverse(result.begin(), result.end());
        return BigInt(neg ? "-" + result : result);
    }
bool operator==(const BigInt &b) const {
        return value == b.value;
    }

    bool operator<(const BigInt &b) const {
        return value.length() < b.value.length() || (value.length() == b.value.length() && value < b.value);
    }

    friend ostream &operator<<(ostream &os, const BigInt &b) {
        os << b.value;
        return os;
    }
};

typedef pair<BigInt, BigInt> Share;

vector<Share> parseJSON(const string &filename, int &k) {
    ifstream file(filename);
    string line;
    vector<Share> shares;

    while (getline(file, line)) {
        if (line.find("\"k\"") != string::npos) {
            size_t pos = line.find(":");
            k = stoi(line.substr(pos + 1));
        }
        if (line.find("[") != string::npos && line.find("[[") == string::npos) {
            size_t start = line.find("[");
            size_t mid = line.find(",", start);
            size_t end = line.find("]", mid);
            string x = line.substr(start + 1, mid - start - 1);
            string y = line.substr(mid + 1, end - mid - 1);
            shares.push_back({BigInt(x), BigInt(y)});
        }
    }
    return shares;
}

void combine(int offset, int k, const vector<Share> &shares, vector<Share> &current, vector<vector<Share>> &combinations) {
    if (k == 0) {
        combinations.push_back(current);
        return;
    }
    for (int i = offset; i <= shares.size() - k; ++i) {
        current.push_back(shares[i]);
        combine(i + 1, k - 1, shares, current, combinations);
        current.pop_back();
    }
}

BigInt lagrangeInterpolation(const vector<Share> &points) {
    BigInt result("0");
    int k = points.size();

    for (int i = 0; i < k; ++i) {
        BigInt xi = points[i].first;
        BigInt yi = points[i].second;
        BigInt num("1"), den("1");

        for (int j = 0; j < k; ++j) {
            if (i != j) {
                BigInt xj = points[j].first;
                num = num * (BigInt("0") + (BigInt("0") - xj));
                den = den * (xi + (BigInt("0") - xj));
            }
        }
        BigInt term = yi * num / stoi(den.value);
        result = result + term;
    }
    return result;
}

int main() {
    int k;
    vector<Share> shares = parseJSON("input.json", k);
    vector<vector<Share>> combinations;
    vector<Share> current;
    combine(0, k, shares, current, combinations);

    map<string, int> freq;
    map<string, vector<vector<Share>>> secretMap;

    for (auto &combo : combinations) {
        BigInt secret = lagrangeInterpolation(combo);
        freq[secret.value]++;
        secretMap[secret.value].push_back(combo);
    }

    string trueSecret;
    int maxCount = 0;
    for (auto &entry : freq) {
        if (entry.second > maxCount) {
            maxCount = entry.second;
            trueSecret = entry.first;
        }
    }

    cout << "Secret: " << trueSecret << endl;

    map<string, int> shareUsage;
    for (auto &combo : secretMap[trueSecret]) {
        for (auto &s : combo) {
            shareUsage[s.first.value + "," + s.second.value]++;
        }
    }

    for (auto &s : shares) {
        string key = s.first.value + "," + s.second.value;
        if (shareUsage.find(key) == shareUsage.end()) {
            cout << "Rogue Share: (" << s.first << ", " << s.second << ")" << endl;
            break;
        }
    }
    return 0;
}