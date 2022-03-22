#ifndef PDP_H
#define PDP_H

#include <stdint.h>
#include <FRC_CAN.h>

#define PDP_APID_Status1 0x50
#define PDP_APID_Status2 0x51
#define PDP_APID_Status3 0x52
#define PDP_APID_StatusEnergy 0x5D
#define PDP_APID_Control1 0x70

#define PDP_DEVTYPE 8L // "Power Distribution Module"
#define PDP_MFG 4L // CTRE

#define PDP_MASK (PDP_DEVTYPE << FRC_DEVICE_SHIFT) | (PDP_MFG << FRC_MANUFACT_SHIFT)

// From allwpilib/hal/src/main/native/athena/CTREPDP.cpp
union PdpStatus1 {
    uint8_t data[8];
    struct Bits {
        unsigned chan1_h8 : 8;
        unsigned chan2_h6 : 6;
        unsigned chan1_l2 : 2;
        unsigned chan3_h4 : 4;
        unsigned chan2_l4 : 4;
        unsigned chan4_h2 : 2;
        unsigned chan3_l6 : 6;
        unsigned chan4_l8 : 8;
        unsigned chan5_h8 : 8;
        unsigned chan6_h6 : 6;
        unsigned chan5_l2 : 2;
        unsigned reserved4 : 4;
        unsigned chan6_l4 : 4;
    } bits;
};

union PdpStatus2 {
    uint8_t data[8];
    struct Bits {
        unsigned chan7_h8 : 8;
        unsigned chan8_h6 : 6;
        unsigned chan7_l2 : 2;
        unsigned chan9_h4 : 4;
        unsigned chan8_l4 : 4;
        unsigned chan10_h2 : 2;
        unsigned chan9_l6 : 6;
        unsigned chan10_l8 : 8;
        unsigned chan11_h8 : 8;
        unsigned chan12_h6 : 6;
        unsigned chan11_l2 : 2;
        unsigned reserved4 : 4;
        unsigned chan12_l4 : 4;
    } bits;
};

union PdpStatus3 {
    uint8_t data[8];
    struct Bits {
        unsigned chan13_h8 : 8;
        unsigned chan14_h6 : 6;
        unsigned chan13_l2 : 2;
        unsigned chan15_h4 : 4;
        unsigned chan14_l4 : 4;
        unsigned chan16_h2 : 2;
        unsigned chan15_l6 : 6;
        unsigned chan16_l8 : 8;
        unsigned internalResBattery_mOhms : 8;
        unsigned busVoltage : 8;
        unsigned temp : 8;
    } bits;
};

union PdpStatusEnergy {
    uint8_t data[8];
    struct Bits {
        unsigned TmeasMs_likelywillbe20ms_ : 8;
        unsigned TotalCurrent_125mAperunit_h8 : 8;
        unsigned Power_125mWperunit_h4 : 4;
        unsigned TotalCurrent_125mAperunit_l4 : 4;
        unsigned Power_125mWperunit_m8 : 8;
        unsigned Energy_125mWPerUnitXTmeas_h4 : 4;
        unsigned Power_125mWperunit_l4 : 4;
        unsigned Energy_125mWPerUnitXTmeas_mh8 : 8;
        unsigned Energy_125mWPerUnitXTmeas_ml8 : 8;
        unsigned Energy_125mWPerUnitXTmeas_l8 : 8;
    } bits;
};

bool PDP_isStatusMsg(uint8_t APID);
bool PDP_isPDP(uint32_t canID);
double PDP_getTemp(uint8_t CANmsg[8]);
double PDP_getVoltage(uint8_t CANmsg[8]);

#endif
