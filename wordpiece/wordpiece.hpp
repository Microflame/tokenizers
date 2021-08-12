#pragma once

#include <vector>
#include <unordered_map>
#include <string_view>

#include <locale>
#include <codecvt>

namespace tokenizers
{

class WordPieceTokenizer
{
public:
    WordPieceTokenizer(const std::string& vocab_path);

    std::vector<int32_t> EncodeAsIds(const std::string& str);

    std::vector<std::string> EncodeAsPieces(const std::string& str);

    int32_t UnkId() { return unk_id_.first; }
    int32_t PadId() { return pad_id_.first; }
    int32_t SepId() { return sep_id_.first; }
    int32_t BosId() { return bos_id_.first; }
    int32_t EosId() { return eos_id_.first; }
    int32_t ClsId() { return cls_id_.first; }

private:
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter_;

    std::vector<std::wstring> id_to_token_;
    std::vector<bool> is_prefix_;

    std::unordered_map<std::wstring_view, int32_t> prefix_to_id_;
    std::unordered_map<std::wstring_view, int32_t> suffix_to_id_;

    std::pair<int32_t, std::string> unk_id_ {0, "[UNK]"};
    std::pair<int32_t, std::string> pad_id_ {0, "[PAD]"};
    std::pair<int32_t, std::string> sep_id_ {0, "[SEP]"};
    std::pair<int32_t, std::string> bos_id_ {0, "[BOS]"};
    std::pair<int32_t, std::string> eos_id_ {0, "[EOS]"};
    std::pair<int32_t, std::string> cls_id_ {0, "[CLS]"};

    bool add_special_tokens_ {true};


    std::vector<int32_t> Tokenize(const std::vector<std::wstring_view>& words);

    size_t Tokenize(const std::wstring_view& word, std::vector<int32_t>* dest);

    // String is split on space symbols.
    // Each Chinese or punctuation symbol is considered as separate word even if not surrounded with spaces.
    std::vector<std::wstring_view> Split(const std::wstring& wstr);

    bool IsChinese(wchar_t ch);

    void TrySetSpecial(std::pair<int32_t, std::string>* token, const std::string& name, int32_t id);
};

} // namespace tokenizers
