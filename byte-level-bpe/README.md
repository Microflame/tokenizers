# Byte Level BPE

This is a simple `c++` implementation of Byte Level BPE tokenizer (aka GPT-2 tokenizer). The goal of this implemetation is to provide a `c++` interface for GPT-2 tokenizer from `huggingface/transformers` and also improve performance.

## Getting the tokenizer

```
cd tokenizers/byte-level-bpe/util
./get_sber.sh
python3 remap-gpt2.py
```

## Building

```
cd ..
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
make
```

## Running

```
./main ../util/gpt-2 -
```

## Perfomance

This implementation is ~10x faster than its `huggingface/transformers` counterpart, however the performance can be further improved by simply using `absl::flat_hash_map` instead of `std::unordered_map`:

```
time ./main ../util/gpt-2 ${STRINGS_FILE} > /dev/null
>>> 5.16 s
```

```
cmake .. -DCMAKE_PREFIX_PATH=${ABSL_PATH} -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
make
time ./main ../util/gpt-2 ${STRINGS_FILE} > /dev/null
>>> 3.84 s
```