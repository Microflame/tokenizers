import json

name_to_id = {}
with open('vocab.json') as f:
    name_to_id = json.load(f)

with open('merges.txt') as f, open('gpt-2.merges', 'w') as fout:
    f.readline() # skip header
    for line in f.readlines():
        first, second = line.split()
        fout.write('%d %d %d\n' % (name_to_id[first],
                                   name_to_id[second],
                                   name_to_id[first + second]))

# https://github.com/huggingface/transformers/blob/master/src/transformers/models/gpt2/tokenization_gpt2.py:65
def bytes_to_unicode():
    bs = (
        list(range(ord("!"), ord("~") + 1)) + list(range(ord("¡"), ord("¬") + 1)) + list(range(ord("®"), ord("ÿ") + 1))
    )
    cs = bs[:]
    n = 0
    for b in range(2 ** 8):
        if b not in bs:
            bs.append(b)
            cs.append(2 ** 8 + n)
            n += 1
    cs = [chr(n) for n in cs]
    return dict(zip(bs, cs))

with open('gpt-2.bytes', 'w') as fout:
    byte2name = bytes_to_unicode()
    for i in range(256):
        fout.write('%d\n' % (name_to_id[byte2name[i]]))

with open('gpt-2.specials', 'w') as fout:
    fout.write('PAD %d\n' % (name_to_id['<pad>']))
    fout.write('BOS %d\n' % (name_to_id['<s>']))
    fout.write('EOS %d\n' % (name_to_id['</s>']))
    fout.write('UNK %d\n' % (name_to_id['<unk>']))
    fout.write('MASK %d\n' % (name_to_id['<mask>']))