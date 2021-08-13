#include <string>
#include <unordered_map>
#include <vector>

namespace tokenizers
{

class ByteLevelBpe
{
public:
    struct PriorityId
    {
        int32_t priority;
        int32_t id;
    };

    struct Specials
    {
        int32_t pad;
        int32_t bos;
        int32_t eos;
        int32_t unk;
        int32_t mask;
    };

    ByteLevelBpe(const std::string& merges_file_path);

    void Tokenize(std::vector<int32_t>* destination, const std::string& str) const;
    void TokenizeSingle(std::vector<int32_t>* destination, const std::string& word) const;

    static std::unordered_map<int64_t, PriorityId> LoadMerges(const std::string& path);
    static std::vector<int32_t> LoadBytesToBpeMapping(const std::string& path);
    static Specials LoadSpecials(const std::string& path);

    template <typename T, typename U>
    static void AppendRange(std::vector<T>* dest, U first, U last)
    {
        while (first != last)
        {
            dest->push_back(first);
            ++first;
        }
    }

    static int64_t JoinIds(int32_t first, int32_t second);
private:
    std::unordered_map<int64_t, PriorityId> merges_;
    std::vector<int32_t> bytes_to_bpe_;
    Specials specials_;
};

} // tokenizers