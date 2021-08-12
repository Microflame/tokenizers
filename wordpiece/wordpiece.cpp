#include "wordpiece.hpp"

#include <fstream>

namespace tokenizers
{

WordPieceTokenizer::WordPieceTokenizer(const std::string& vocab_path)
{
    static_assert(sizeof(wchar_t) == 4);
    
    std::fstream vocab_file(vocab_path);
    std::string line;
    id_to_token_.clear();
    while (std::getline(vocab_file, line))
    {
        TrySetSpecial(&unk_id_, line, id_to_token_.size());
        TrySetSpecial(&pad_id_, line, id_to_token_.size());
        TrySetSpecial(&sep_id_, line, id_to_token_.size());
        TrySetSpecial(&bos_id_, line, id_to_token_.size());
        TrySetSpecial(&eos_id_, line, id_to_token_.size());
        TrySetSpecial(&cls_id_, line, id_to_token_.size());

        if (line.size() > 2 && line[0] == '#' && line[1] == '#')
        {
            is_prefix_.push_back(false);
            line = line.substr(2);
        }
        else
        {
            is_prefix_.push_back(true);
        }
        id_to_token_.push_back(converter_.from_bytes(line));
    }
    for (size_t i = 0; i < id_to_token_.size(); i++)
    {
        std::wstring_view view(id_to_token_[i].data(), id_to_token_[i].size());
        if (is_prefix_[i])
        {
            prefix_to_id_[view] = i;
        }
        else
        {
            suffix_to_id_[view] = i;
        }
    }
}

std::vector<int32_t> WordPieceTokenizer::EncodeAsIds(const std::string& str)
{
    std::wstring wstr = converter_.from_bytes(str);
    std::vector<std::wstring_view> view = Split(wstr);
    std::vector<int32_t> res = Tokenize(view);
    return res;
}

std::vector<std::string> WordPieceTokenizer::EncodeAsPieces(const std::string& str)
{
    std::vector<std::string> res;
    std::vector<int32_t> ids = EncodeAsIds(str);
    for (int32_t id: ids)
    {
        wchar_t* first = id_to_token_[id].data();
        wchar_t* last = id_to_token_[id].data() + id_to_token_[id].size();
        std::string piece = is_prefix_[id] ? "" : "##";
        piece += converter_.to_bytes(first, last);
        res.push_back(piece);
    }
    return res;
}

std::vector<int32_t> WordPieceTokenizer::Tokenize(const std::vector<std::wstring_view>& words)
{
    std::vector<int32_t> res;

    if (add_special_tokens_)
    {
        res.push_back(ClsId());
    }

    for (const std::wstring_view& word: words)
    {
        Tokenize(word, &res);
    }

    if (add_special_tokens_)
    {
        res.push_back(SepId());
    }

    return res;
}

size_t WordPieceTokenizer::Tokenize(const std::wstring_view& word, std::vector<int32_t>* dest)
{
    const wchar_t* start = word.data();
    const wchar_t* end = start + word.size();
    size_t num_encoded = 0;
    while (start < end)
    {
        const wchar_t* token_end = end;
        while (start < token_end)
        {
            auto& map = num_encoded == 0 ? prefix_to_id_ : suffix_to_id_;
            std::wstring_view token_view(start, token_end - start);
            auto it = map.find(token_view);
            
            if (it != map.end())
            {
                dest->push_back(it->second);
                ++num_encoded;
                start = token_end;
                break;
            }
            --token_end;
            if (token_end == start)
            {
                dest->resize(dest->size() - num_encoded);
                dest->push_back(UnkId());
                return 1;
            }
        }
    }
    return num_encoded;
}

// String is split on space symbols.
// Each Chinese or punctuation symbol is considered as separate word even if not surrounded with spaces.
std::vector<std::wstring_view> WordPieceTokenizer::Split(const std::wstring& wstr)
{
    std::vector<std::wstring_view> res;
    const wchar_t* start = wstr.data();
    const wchar_t* cur = wstr.data();
    for (; cur < wstr.data() + wstr.size(); ++cur)
    {
        if (std::iswspace(*cur))
        {
            if (cur > start)
            {
                res.emplace_back(start, cur - start);
            }
            start = cur + 1;
        }
        else if (std::iswpunct(*cur) || IsChinese(*cur))
        {
            if (cur > start)
            {
                res.emplace_back(start, cur - start);
            }
            res.emplace_back(cur, 1);
            start = cur + 1;
        }
    }
    if (cur > start)
    {
        res.emplace_back(start, cur - start);
    }
    return res;
}

bool WordPieceTokenizer::IsChinese(wchar_t ch)
{
    if ((ch >= 0x4E00 && ch <= 0x9FFF) ||
        (ch >= 0x3400 && ch <= 0x4DBF) ||
        (ch >= 0x20000 && ch <= 0x2A6DF) ||  
        (ch >= 0x2A700 && ch <= 0x2B73F) ||
        (ch >= 0x2B740 && ch <= 0x2B81F) ||
        (ch >= 0x2B820 && ch <= 0x2CEAF) ||
        (ch >= 0xF900 && ch <= 0xFAFF) ||
        (ch >= 0x2F800 && ch <= 0x2FA1F))
    {
        return true;
    }
    return false;
}

void WordPieceTokenizer::TrySetSpecial(std::pair<int32_t, std::string>* token, const std::string& name, int32_t id)
{
    if (name == token->second)
    {
        token->first = id;
    }
}

} // tokenizers