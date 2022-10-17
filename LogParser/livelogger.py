import serial
from logparser import parseCANID, constructCANID, mfglut, devtylut
import ctypes as ct

ser = serial.Serial("COM7")
ser.baudrate = 115200
ser.read_all()
ser.reset_output_buffer()
ser.reset_input_buffer()


class Msg(ct.Union):
    _fields_ = (("bytes", ct.c_char * 8),
                ("short", ct.c_short))


freqs = [0 for i in range(8*8)]
last = [0 for i in range(8)]

while True:
    ser_bytes = ser.readline()
    mesgcsv = ser_bytes[0:len(ser_bytes)-2].decode("utf-8")
    mesgcsv = [int(p) for p in mesgcsv.split(',')]
    timestamp = mesgcsv[0]
    (dvctype, mfgcode, apiclass, apiindex,
     devcnum, apid) = parseCANID(mesgcsv[1])
    data = mesgcsv[2:]

    # if (devcnum != 10):
    #     continue

    # Bits:
    # print("|".join([bin(d)[2:].ljust(8, '0') for d in data]))

    # Full message
    print(
        f"TYPE = {devtylut[dvctype]}, MFG = {mfglut[mfgcode]}, NUM = {devcnum}, APID = {apid} : {data}")
