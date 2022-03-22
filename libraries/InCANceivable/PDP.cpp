#include <pdp.h>

// Return the "voltage" measurement from the Vex PDP (0-100)
double PDP_getVoltage(uint8_t CANmsg[8]) {
    PdpStatus3 stat;
    for (int i = 0; i < 8; ++i) {
        stat.data[i] = CANmsg[i];
    }
    return stat.bits.busVoltage * 0.5 + 4.0;
}

// Return the temperature measurement from the PDP
double PDP_getTemp(uint8_t CANmsg[8]) {
    PdpStatus3 stat;
    for (int i = 0; i < 8; ++i) {
        stat.data[i] = CANmsg[i];
    }
    return stat.bits.temp * 1.03250836957542 - 67.8564500484966;
}

// Checks if a PDP message is a "Status3" message (containing voltage and temp, other API IDs send per-channel current measurements)
bool PDP_isStatusMsg(uint8_t APID) {
    return APID == PDP_APID_Status3;
}

// Check if a CAN message ID is coming from the PDP
bool PDP_isPDP(uint32_t canID) {
    return (PDP_MASK & canID) == PDP_MASK;
}
