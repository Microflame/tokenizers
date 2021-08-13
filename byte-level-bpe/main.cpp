#include <iostream>
#include <fstream>
#include <fstream>
#include <exception>
#include <iterator>

#include "byte-level-bpe.hpp"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        throw std::runtime_error("Wrong number of arguments");
    }

    std::string merges_path = argv[1];
    std::string strings_path = argv[2];

    tokenizers::ByteLevelBpe tokenizer(merges_path);

    std::istream* is;
    std::ifstream ifs;
    if (strings_path == "-")
    {
        is = &std::cin;
    }
    else
    {
        ifs.open(strings_path);
        if (!ifs.is_open())
        {
            throw std::runtime_error("Can not open strings file");
        }
        is = &ifs;
    }

    std::string str;
    std::vector<int32_t> ids;
    while (std::getline(*is, str))
    {
        tokenizer.Tokenize(&ids, str);
        std::copy(ids.begin(), ids.end(), std::ostream_iterator<int32_t>(std::cout, " "));
    }

    return 0;
}