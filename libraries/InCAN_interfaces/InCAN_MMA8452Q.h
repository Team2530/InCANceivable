#ifndef INCAN_MMA8452Q_INCLUDED
#define INCAN_MMA8452Q_INCLUDED


int MMA8452Q_init();
int MMA8452Q_config(unsigned long int *canId, int length, char* buf);
int MMA8452Q_update();
int MMA8452Q_emitVals();
// emitted values are in line with INCAN_ACC_DATA
// buf[4-7] bytes are int "what" 
// buf[0-3] bytes are float* value
// if we do we will average x,y,z then do geometry on the average 
// C.f. Rician noise
#endif //  INCAN_MMA8452Q_INCLUDED
