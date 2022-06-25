from math import floor
from typing import DefaultDict
from logparser import parseCANID, constructCANID, PDPStatusMsg1
import pickle

messages = [[int(n) for n in line.strip().split(',')]
            for line in open("talon.csv").readlines()]


def bits(message):
    return [((message[floor(i/8)] >> (7-(i % 8))) &
            1) for i in range(len(message)*8)]


def switched(old, bits):
    return [(old[i] != bits[i]) and bits[i] for i in range(len(bits))]


chgcts = [0 for x in range(8*8)]
last = bits(messages[0])


print(bits([4, 1]))

for message in messages:
    mbits = bits(message)
    c = switched(last, mbits)
    chgcts = [chgcts[i] + c[i] for i in range(len(mbits))]
    # print(message[1] | ((message[0] & 0xF) << 4))
print(chgcts)

# apid: 97
# combined: [132, 0, 100, 186, 0, 0, 0, 255]
# apid: 2
# combined: [0, 0, 0, 0, 0, 11, 0, 0]
# apid: 1
# combined: [0, 0, 136, 0, 0, 0, 0, 0]
# apid: 144
# combined: [0, 0, 255, 0, 0, 0, 0, 0]
# apid: 10
# combined: [2, 1, 9, 0, 0, 0, 0, 0]
# apid: 228
# combined: [2, 62, 0, 170, 170, 170, 170, 170]
# apid: 0
# combined: [0, 0, 0, 0, 0, 0, 0, 0]
