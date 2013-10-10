#include <algorithm>
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

    ini_doc() = default;
    ini_doc(ini_section default_section)
        : ini_doc{move(default_section), {}}
    {}
    ini_doc(unordered_map<string, ini_section> sections)
        : ini_doc{{}, move(sections)}
    {}
    ini_doc(ini_section default_section, unordered_map<string, ini_section> sections)
        : default_section{move(default_section)}, sections{move(sections)}
    {}

#if CAN_COPY_DOC
    ini_doc(const ini_doc& other) = default;
    ini_doc& operator=(const ini_doc& other) = default;
#else
    ini_doc(const ini_doc& other) = delete;
    ini_doc& operator=(const ini_doc& other) = default;
#endif

#if _MSC_VER < 1900 // then we don't have move defaulting
    ini_doc(ini_doc&& other) throw()
    {
        swap(default_section, other.default_section);
        swap(sections, other.sections);
    }

    ini_doc& operator=(ini_doc&& other) throw()
    {
        swap(*this, other);
        return *this;
    }
#else
    ini_doc(ini_doc&& other) noexcept = default;
    ini_doc& operator=(ini_doc&& other) noexcept = default;
#endif
};

inline bool operator==(const ini_doc& l, const ini_doc& r)
{
    if (l.default_section != r.default_section)
        return false;
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

template <typename It>
auto trim_start(It first, It last) -> It
{
    return find_if_not(first, last, isspace);
}

template <typename It>
auto trim_end(It first, It last) -> It
{
    return find_if_not(reverse_iterator<It>(last), reverse_iterator<It>(first), isspace).base();
}

template <typename It>
auto trim(It first, It last) -> string
{
    first = trim_start(first, last);
    last = trim_end(first, last);
    return {first, last};
}

auto trim(const string& s) -> string
{
    return trim(begin(s), end(s));
}

auto parse_ini_doc(istream& is) -> ini_doc
{
    auto result = ini_doc{};
    auto line = string{};

    auto current_section = &result.default_section;

    while (getline(is, line))
    {
        auto first = begin(line);
        auto last = end(line);

        // Remove comments
        last = find(first, last, ';');

        // Remove whitespace
        first = trim_start(first, last);
        last = trim_end(first, last);

        // Skip empty lines
        if (first == last)
        {
            continue;
        }

        // Match section headers
        if (*first == '[' && *prev(last) == ']')
        {
            current_section = &result.sections[string{next(first), prev(last)}];
            continue;
        }

        // Match values
        auto split = find(first, last, '=');
        if (split != last)
        {
            auto name = trim(first, split);
            auto value = trim(next(split), last);

            current_section->emplace(move(name), move(value));
            continue;
        }

        // No match!
    }

    return result;
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
    cerr << "input:===============================\n" << input << "\n";
    cerr << "expected:============================\n" << expected << "\n";
    cerr << "actual:==============================\n" << actual << "\n";
    abort();
}

void test_empty()
{
    auto input = "";
    auto expected = ini_doc{};
    test_impl("empty", input, expected);
}

void test_default_section()
{
    auto input = R"(
name1=value1
name2=value2
)";
    auto expected = ini_doc{
        ini_section{
            {"name1", "value1"},
            {"name2", "value2"},
        }
    };
    test_impl("default section", input, expected);
}

void test_comments()
{
    auto input = R"(
;name1=value1
name2=value2 ; a comment
)";
    auto expected = ini_doc{
        ini_section{
            {"name2", "value2"},
        }
    };
    test_impl("comments", input, expected);
}

void test_single_section()
{
    auto input = R"(
[section_name]
name1=value1
name2=value2
)";
    auto expected = ini_doc{
        {
            {
                "section_name",
                ini_section{
                    {"name1", "value1"},
                    {"name2", "value2"},
                }
            }
        }
    };
    test_impl("comments", input, expected);
}

void test()
{
    test_empty();
    test_default_section();
    test_comments();
    test_single_section();
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
