from typing import DefaultDict
from logparser import parseCANID, constructCANID
import pickle

messages = [[int(n) for n in line.strip().split(',')]
            for line in open("talon.csv").readlines()]

for message in messages:
    print(((message[0] & 0b1111111) << 4) | (message[1] >> 4))

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
