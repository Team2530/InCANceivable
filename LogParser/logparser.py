# Device and manufacturer codes of 1 is RIO
from ast import parse
from pprint import pprint
import matplotlib.pyplot as plt
import numpy as np
from ctypes import *
import json

RIO_MASK = (1 << 24) | (1 << 16)

mfglut = [
    "Broadcast",
    "NI",
    "Luminary",
    "DEKA",
    "CTRE",
    "REV",
    "Grapple",
    "MindSensors",
    "Custom",
    "Kauai Labs",
    "Copperforge",
    "Playing With Fusion",
    "Studica",
]

devtylut = [
    "Broadcast Message",
    "Robot Controller",
    "Motor Controller",
    "Relay Controller",
    "Gyro Sensor",
    "Accelerometer",
    "Ultrasonic Sensor",
    "Gear Tooth Sensor",
    "Power Distribution Module",
    "Pneumatics Controller",
    "Miscellaneous",
    "IO Breakout",
    "Reserved",
]


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
    apiid = (canid >> 6) & 0xFF
    mfgcode = (canid >> 16) & 0xFF
    dvctype = (canid >> 24) & 0x1F

    # (devctype, mfg_code, apiclass, apiid, devcnum)
    return (dvctype, mfgcode, apiclass, apiindex, devcnum, apiid)


def isRIO(canid):
    return (canid & RIO_MASK) == RIO_MASK


def isPDP(canid):
    dvcty, mfg, _, _, _, apiid = parseCANID(canid)
    return dvcty == 8 and mfg == 4


def getPDPVoltage(data):
    return data[6] * 0.5 + 4.0

# RIO ID
# print(parseCANID(0x1011840), "\n")

# (Device type, Manufacturer, Device num)
#
# (0, 4, 63),   Broadcast message?
# (1, 1, 0),    RIO (NI)
# (1, 4, 8),    "Robot Controller" (CTRE)
# (2, 4, 1),    Drive motor FL (TalonFX)
# (2, 4, 2),    Drive motor FR (TalonFX)
# (2, 4, 3),    Drive motor BL (TalonFX)
# (2, 4, 4),    Drive motor BR (TalonFX)
# (2, 4, 5),    *Ghost CTRE Motor*
# (2, 4, 7),    *Ghost CTRE Motor*
# (2, 4, 8),    *Ghost CTRE Motor*
# (2, 4, 10),   Lower intake (TalonFX)
# (2, 4, 20),   Climber right (TalonSRX)
# (8, 4, 0),    PDP (CTRE)
# (8, 4, 6),    Other "power distribution module" (VRM<->PDP?)
# (10, 8, 3)    InCANceivable


if __name__ == "__main__":
    lines = [[int(i) for i in line.split(',')]
             for line in open("logs/LOG_13.CSV").readlines()[1:]]

    devices = set()
    voltages = []
    channelcurrents = [[]] * 16

    class PDPStatusMsg1(Structure):
        _pack_ = 1
        _fields_ = [("chan1_h8", c_uint, 8),
                    ("chan2_h6", c_uint, 6),
                    ("chan1_l2", c_uint, 2),
                    ("chan3_h4", c_uint, 4),
                    ("chan2_l4", c_uint, 4),
                    ("chan4_h2", c_uint, 2),
                    ("chan3_l6", c_uint, 6),
                    ("chan4_l8", c_uint, 8),
                    ("chan5_h8", c_uint, 8),
                    ("chan6_h6", c_uint, 6),
                    ("chan5_l2", c_uint, 2),
                    ("reserved4", c_uint, 4),
                    ("chan6_l4", c_uint, 4)]

    class PDPStatusMsg3(Structure):
        _pack_ = 1
        _fields_ = [("chan13_h8", c_uint, 8),
                    ("chan14_h6", c_uint, 6),
                    ("chan13_l2", c_uint, 2),
                    ("chan15_h4", c_uint, 4),
                    ("chan14_l4", c_uint, 4),
                    ("chan16_h2", c_uint, 2),
                    ("chan15_l6", c_uint, 6),
                    ("chan16_l8", c_uint, 8),
                    ("internalResBattery_mOhms", c_uint, 8),
                    ("busVoltage", c_uint, 8),
                    ("temp ", c_uint, 8)]

    def readPDPChannels(apiid, data):
        global channelcurrents
        if (apiid == 0x50):  # Channels 1-6
            stat = PDPStatusMsg1.from_buffer(bytearray(data))
            channelcurrents[0].append((stat.chan1_h8 << 2) | (stat.chan1_l2))
            channelcurrents[1].append((stat.chan2_h6 << 4) | (stat.chan2_l4))
            channelcurrents[2].append((stat.chan3_h4 << 6) | (stat.chan3_l6))
            channelcurrents[3].append((stat.chan4_h2 << 8) | (stat.chan4_l8))
            channelcurrents[4].append((stat.chan5_h8 << 2) | (stat.chan5_l2))
            channelcurrents[5].append((stat.chan6_h6 << 4) | (stat.chan6_l4))
        elif (apiid == 0x51):  # Channels 7-12
            stat = PDPStatusMsg1.from_buffer(bytearray(data))
            channelcurrents[0+6].append((stat.chan1_h8 << 2) | (stat.chan1_l2))
            channelcurrents[1+6].append((stat.chan2_h6 << 4) | (stat.chan2_l4))
            channelcurrents[2+6].append((stat.chan3_h4 << 6) | (stat.chan3_l6))
            channelcurrents[3+6].append((stat.chan4_h2 << 8) | (stat.chan4_l8))
            channelcurrents[4+6].append((stat.chan5_h8 << 2) | (stat.chan5_l2))
            channelcurrents[5+6].append((stat.chan6_h6 << 4) | (stat.chan6_l4))
        elif (apiid == 0x52):  # Channels 13-16
            stat = PDPStatusMsg3.from_buffer(bytearray(data))
            channelcurrents[0 +
                            12].append((stat.chan13_h8 << 2) | (stat.chan13_l2))
            channelcurrents[1 +
                            12].append((stat.chan14_h6 << 4) | (stat.chan14_l4))
            channelcurrents[2 +
                            12].append((stat.chan15_h4 << 6) | (stat.chan15_l6))
            channelcurrents[3 +
                            12].append((stat.chan16_h2 << 8) | (stat.chan16_l8))

    for i in range(len(lines)):
        p = parseCANID(lines[i][1])
        datalen = lines[i][2]
        data = lines[i][3:]
        timestamp = lines[i][0]

        # Device type, manufacturer, and num
        dvc = (p[0], p[1], p[4])
        devices.add(dvc)

        if isPDP(lines[i][1]):
            if (p[-1] == 0x52):
                voltages.append((timestamp, getPDPVoltage(data)))

            # Read channel currents
            readPDPChannels(p[-1], data)
        # if isRIO(lines[i][1]) and datalen == 8:
        #     # print(data[0:datalen])
        #     heartbeat = {
        #         "alliance": "red" if (data[4] & 1) else "blue",
        #         "fms_enabled": bool(data[4] & 2),
        #         "autonomous": bool(data[4] & 4),
        #         "test": bool(data[4] & 8),
        #         "enabled": bool(data[4] & 16),  # "WatchdogEnabled"
        #         "reserved": (data[4] >> 5) & 0x7,
        #     }
        #     print("--- Heartbeat ---")
        #     print("Enabled:", heartbeat["enabled"], "\n")
        # elif p[0] == 10 and p[1] == 8:
        #     print("--- InCANceivable ---")
        #     print("Ball states:", [
        #         ["IDK", "Red", "Green", "Blue", "None"][ball] for ball in data[:4]], "\n")
        # elif p[0] == 0:  # Broadcast message
        #     print("--- Broadcast ---")
        #     print(data)
        #     print()

    open("voltages.csv",
         'w').writelines([f"{v[0]},{v[1]}\n" for v in voltages])

    json.dump(channelcurrents, open("channelcurrents.json", "w"), indent=4)

    pprint(devices)
