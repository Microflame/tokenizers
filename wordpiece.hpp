#include <iostream>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <string_view>

#include <locale>
#include <codecvt>

namespace neuro
{
namespace tokenizer
{

class WordPieceTokenizer
{
public:
    WordPieceTokenizer(const std::string& vocab_path)
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
            id_to_token_.push_back(converter_.from_bytes(line));
        }
        for (size_t i = 0; i < id_to_token_.size(); i++)
        {
            std::wstring_view view(id_to_token_[i].data(), id_to_token_[i].size());
            token_to_id_[view] = i;
        }
        
    }

    std::vector<int32_t> EncodeAsIds(const std::string& str)
    {
        std::wstring wstr = converter_.from_bytes(str);
        std::vector<std::wstring_view> view = Split(wstr);
        std::vector<int32_t> res = Tokenize(view);
        return res;
    }

    std::vector<std::string> EncodeAsPieces(const std::string& str)
    {
        std::vector<std::string> res;
        std::vector<int32_t> ids = EncodeAsIds(str);
        for (int32_t id: ids)
        {
            wchar_t* first = id_to_token_[id].data();
            wchar_t* last = id_to_token_[id].data() + id_to_token_[id].size();
            res.push_back(converter_.to_bytes(first, last));
        }
        return res;
    }

    int32_t UnkId() { return unk_id_.first; }
    int32_t PadId() { return pad_id_.first; }
    int32_t SepId() { return sep_id_.first; }
    int32_t BosId() { return bos_id_.first; }
    int32_t EosId() { return eos_id_.first; }

private:
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter_;
    std::vector<std::wstring> id_to_token_;
    std::unordered_map<std::wstring_view, int32_t> token_to_id_;
    std::pair<int32_t, std::string> unk_id_ {0, "[UNK]"};
    std::pair<int32_t, std::string> pad_id_ {0, "[PAD]"};
    std::pair<int32_t, std::string> sep_id_ {0, "[SEP]"};
    std::pair<int32_t, std::string> bos_id_ {0, "[BOS]"};
    std::pair<int32_t, std::string> eos_id_ {0, "[EOS]"};

    std::vector<int32_t> Tokenize(const std::vector<std::wstring_view>& words)
    {
        std::vector<int32_t> res;
        for (const std::wstring_view& word: words)
        {
            Tokenize(word, &res);
        }
        return res;
    }

    size_t Tokenize(const std::wstring_view& word, std::vector<int32_t>* dest)
    {
        const wchar_t* start = word.data();
        const wchar_t* end = start + word.size();
        size_t num_encoded = 0;
        while (start < end)
        {
            const wchar_t* token_end = end;
            while (start < token_end)
            {
                std::unordered_map<std::wstring_view, int32_t>::iterator it;
                if (num_encoded == 0)
                {
                    std::wstring_view token_view(start, token_end - start);
                    it = token_to_id_.find(token_view);
                }
                else
                {
                    std::wstring suffix = L"##" + std::wstring(start, token_end);
                    std::wstring_view token_view(suffix.data(), suffix.size());
                    it = token_to_id_.find(token_view);
                }
                
                if (it != token_to_id_.end())
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
    std::vector<std::wstring_view> Split(const std::wstring& wstr)
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

    bool IsChinese(wchar_t ch)
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

    void TrySetSpecial(std::pair<int32_t, std::string>* token, const std::string& name, int32_t id)
    {
        if (name == token->second)
        {
            token->first = id;
        }
    }
};

} // namespace tokenizer
} // namespace neuro
