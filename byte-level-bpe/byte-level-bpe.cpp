#include "byte-level-bpe.hpp"

#include <fstream>
#include <sstream>
#include <regex>
#include <exception>
#include <cstring>

#include <iostream>

namespace tokenizers
{

ByteLevelBpe::ByteLevelBpe(const std::string& bpe_path) :
    merges_(LoadMerges(bpe_path + ".merges")),
    bytes_to_bpe_(LoadBytesToBpeMapping(bpe_path + ".bytes")),
    specials_(LoadSpecials(bpe_path + ".specials"))
{

}

void ByteLevelBpe::Tokenize(std::vector<int32_t>* destination, const std::string& str) const
{
    destination->clear();
    std::regex expr("'s|'t|'re|'ve|'m|'ll|'d| ?\\p\\{L\\}+| ?\\p\\{N\\}+| ?[^\\s\\p\\{L\\}\\p\\{N\\}]+|\\s+(?!\\S)|\\s+");
    std::smatch match;

    auto begin = str.cbegin();
    while (std::regex_search(begin, str.cend(), match, expr))
    {
        TokenizeSingle(destination, match[0]);
        begin = match.suffix().first;
    }
}

void ByteLevelBpe::TokenizeSingle(std::vector<int32_t>* destination, const std::string& word) const
{
    std::vector<int32_t> word_bpe;
    for (unsigned char c: word)
    {
        word_bpe.push_back(bytes_to_bpe_[c]);
    }

    while (true)
    {
        ssize_t best_pair = -1;
        PriorityId best_priority_id = {INT32_MAX, -1};
        for (size_t i = 0; i < word_bpe.size() - 1; i++)
        {
            int64_t pair = JoinIds(word_bpe[i], word_bpe[i + 1]);
            auto it = merges_.find(pair);
            if (it == merges_.end())
            {
                continue;
            }
            PriorityId priority_id = it->second;
            if (priority_id.priority < best_priority_id.priority)
            {
                best_priority_id = priority_id;
                best_pair = i;
            }
        }
        if (best_pair == -1)
        {
            break;
        }

        word_bpe[best_pair] = best_priority_id.id;
        memmove(&word_bpe[best_pair + 1], &word_bpe[best_pair + 2],
                (word_bpe.size() - best_pair - 2) * sizeof(int32_t));
        word_bpe.resize(word_bpe.size() - 1);
    }
    destination->insert(destination->end(), word_bpe.begin(), word_bpe.end());
}

std::unordered_map<int64_t, ByteLevelBpe::PriorityId> ByteLevelBpe::LoadMerges(const std::string& path)
{
    std::unordered_map<int64_t, ByteLevelBpe::PriorityId> res;

    std::ifstream merges(path);
    if (!merges.is_open())
    {
        throw std::runtime_error("Can not open merges file: '" + path + "'");
    }

    std::string line;
    int32_t priority = 0;
    while (std::getline(merges, line))
    {
        std::stringstream ss(line);

        int32_t first;
        int32_t second;
        int32_t result;
        ss >> first;
        ss >> second;
        ss >> result;
        res[JoinIds(first, second)] = {priority, result};

        ++priority;
    }

    return res;
}

std::vector<int32_t> ByteLevelBpe::LoadBytesToBpeMapping(const std::string& path)
{
    std::vector<int32_t> res(256);
    std::ifstream bytes(path);
    if (!bytes.is_open())
    {
        throw std::runtime_error("Can not open bytes file: '" + path + "'");
    }

    for (size_t i = 0; i < 256; i++)
    {
        bytes >> res[i];
    }
    
    return res;
}

ByteLevelBpe::Specials ByteLevelBpe::LoadSpecials(const std::string& path)
{
    Specials res;

    std::ifstream specials_file(path);
    if (!specials_file.is_open())
    {
        throw std::runtime_error("Can not open specials file: '" + path + "'");
    }

    std::string line;
    while (std::getline(specials_file, line))
    {
        std::stringstream ss(line);

        std::string name;
        int32_t id;
        ss >> name;
        ss >> id;

        if (name == "PAD")
        {
            res.pad = id;
        }
        else if (name == "BOS")
        {
            res.bos = id;
        }
        else if (name == "EOS")
        {
            res.eos = id;
        }
        else if (name == "UNK")
        {
            res.unk = id;
        }
        else if (name == "MASK")
        {
            res.mask = id;
        }
        else
        {
            throw std::runtime_error("Bad special name: '" + name + "'");
        }
    }

    return res;
}

int64_t ByteLevelBpe::JoinIds(int32_t first, int32_t second)
{
    int64_t res = first;
    res <<= 32;
    res += second;
    return res;
}

} // tokenizers