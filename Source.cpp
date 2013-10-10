#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>

using namespace std;

using ini_section = unordered_map<string, string>;

struct ini_doc
{
    ini_section default_section;
    unordered_map<string, ini_section> sections;
};

inline bool operator==(const ini_doc& l, const ini_doc& r)
{
    return true;
}

inline bool operator!=(const ini_doc& l, const ini_doc& r)
{
    return !(l == r);
}

inline ostream& operator<<(ostream& os, const ini_doc& doc)
{
    for (auto const& item : doc.default_section)
        os << item.first << '=' << item.second << '\n';
    for (auto const& section : doc.sections)
    {
        os << '[' << section.first << "]\n";
        for (auto item : section.second)
            os << item.first << '=' << item.second << '\n';
    }
    return os;
}

auto parse_ini_doc(istream& is) -> ini_doc
{
    return {};
}

void test_impl(const char* name, const char* input, const ini_doc& expected)
{
    auto actual = parse_ini_doc(istringstream{input});

    if (actual == expected)
    {
        cout << "TEST PASS: " << name << "\n";
        return;
    }
    cerr << "TEST FAIL: " << name << "\n";
    cerr << "input:\n" << input << "\n";
    cerr << "expected:\n" << expected << "\n";
    cerr << "actual:\n" << actual << "\n";
    abort();
}

void test_empty()
{
    auto input = "";
    auto expected = ini_doc{};
    test_impl("empty", input, expected);
}

void test()
{
    test_empty();
}

int main(int argc, const char* argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: ini INIFILE\n";
        return 1;
    }

    if (argv[1] == string{"--test"})
    {
        test();
        return 0;
    }

    auto in = ifstream{argv[1]};
    if (!in.is_open())
    {
        cerr << "Could not open '" << argv[1] << "'\n";
        return 2;
    }

}
