from ast import parse
import csv

# Device and manufacturer codes of 1 is RIO
RIO_MASK = (1 << 24) | (1 << 16)

lines = [[int(i) for i in line.split(',')]
         for line in open("logs/CANLOG_7.CSV").readlines()]


# Constructs a 29-bit CAN ID from different parts (device type, manufacturer code, API class, API ID, device number)
def constructCANID(dvctype, mfgcode, apiclass, apiindex, devcnum):
    return (devcnum & 0x3F) | ((apiindex & 0xF) << 6) | ((apiclass & 0x3F) << 10) | ((mfgcode & 0xFF) << 16) | ((dvctype & 0x1F) << 24)


# Parses a FRC-CAN message ID, returns a tuple of the different parts of the ID
def parseCANID(canid):
    # print("CAN ID:    ", "{0:b}".format(canid).rjust(29, '0'))
    # print("           ", "|" * 29)
    # print("Device num:", "{0:b}".format(0x3F).rjust(29, '0'))
    # print("API index: ", "{0:b}".format(0xF << 6).rjust(29, '0'))
    # print("API class: ", "{0:b}".format(0x3F << 10).rjust(29, '0'))
    # print("Mfg. code: ", "{0:b}".format(0xFF << 16).rjust(29, '0'))
    # print("Dvc. type: ", "{0:b}".format(0x1F << 24).rjust(29, '0'))
    devcnum = canid & 0x3F
    apiindex = (canid >> 6) & 0xF
    apiclass = (canid >> 10) & 0x3F
    mfgcode = (canid >> 16) & 0xFF
    dvctype = (canid >> 24) & 0x1F

    # (devctype, mfg_code, apiclass, apiid, devcnum)
    return (dvctype, mfgcode, apiclass, apiindex, devcnum)


def isRIO(canid):
    return (canid & RIO_MASK) == RIO_MASK


print(parseCANID(0x1011840), "\n")


for i in range(len(lines)):
    p = parseCANID(lines[i][1])
    datalen = lines[i][2]
    data = lines[i][3:(3+datalen)]

    if isRIO(lines[i][1]) and datalen == 8:
        # print(data[0:datalen])
        heartbeat = {
            "alliance": "red" if (data[4] & 1) else "blue",
            "fms_enabled": bool(data[4] & 2),
            "autonomous": bool(data[4] & 4),
            "test": bool(data[4] & 8),
            "enabled": bool(data[4] & 16),  # "WatchdogEnabled"
            "reserved": (data[4] >> 5) & 0x7,
        }
        print("Heartbeat")
        print("Enabled:", heartbeat["enabled"], "\n")
    elif p[0] == 10 and p[1] == 8:
        print("InCANceivable!")
        print("Ball states:", [
            ["IDK", "Red", "Green", "Blue", "None"][ball] for ball in data])
