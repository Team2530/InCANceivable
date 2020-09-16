#include <InCAN_MMA8452Q.h>

#define MMA8452Q_BUFSZ 32
float mma8452Q_bufX[MMA8452Q_BUFSZ];
float mma8452Q_bufY[MMA8452Q_BUFSZ];
float mma8452Q_bufZ[MMA8452Q_BUFSZ];
unsigned long int mma8452Q_micros[MMA8452Q_BUFSZ];
int mma8452Q_index=0;
long int mma8452Q_timeWindow=10000; // in micros //  
int sendXYZ=1;
int sendGrad=1;
int sendAngle=0;

char CANbuf[8];
int CANlen=8;

MMA8452Q accel;

int MMA8452Q_init()
{
  int ret=0;
  int i;
  /* zero the data blocks */
  for (i=0;i<MMA8452Q_BUFSZ;i++)
    {
      mma8452Q_micros(i)=0;
      mma8452Q_bufX(i)=0;
      mma8452Q_bufY(i)=0;
      mma8452Q_bufZ(i)=0;
    }
  Wire.begin(); // can multiple modules do Wire.begin() w/o walking on each other? 
  while ((accel.begin() == false ) && ret<100)
    {
      delay(10);
    Serial.println("mma8452Q not connected?")
      ret++;
      }
  // any default configuration for the accel goes here. 
  return(ret);
}


int MMA8452Q_config(unsigned long int *canId, int length, char* buf)
{
  int ret=0;
  /* if we had configuration options we would deconstruct the canId
     to figure out what to do ;  for now we provide no options
   */ 
  return(ret);
}


int MMA8452Q_update()
{
  int ret=0;
  /* read the sensors and update the internal data block */
  if (accel.available())  
    {
  mma8452Q_bufX(mma8452Q_index)=accel.getCalculatedX();
  mma8452Q_bufY(mma8452Q_index)=accel.getCalculatedY();
  mma8452Q_bufZ(mma8452Q_index)=accel.getCalculatedZ();
  mma8452Q_micros(mma8452Q_index)=micros();
  mma8452Q_index= (mma8452Q_index++) % MMA8452Q_BUFSZ;
    }
  else{
    ret=1;
  }
  return(ret);
}

int MMA8452Q_emitVals()
{
  int ret=0;
  int i;
  int nSum=0;
  float sumX=0;
  float sumY=0;
  float sumZ=0;
  float aveX; 
  float aveY;
  float aveZ;
  float mag;
  float slope;
  float angle;
  unsigned long maxTime=0;
  unsigned long int canId;
  unsigned long int hwJumpers=0;
    /* build up out output CAN ID, fill in the buf()*/
  GET THE CANBED number from pins 9,10 
  use those to set the hw numbers
  canId= INCAN_MASK | INCAN_CL_ACC < FRC_CLASS_SHIFT | INCAN_ACC_DATA < FRC_CLINDEX_SHIFT | hwJumpers ;
   //loop over the micros and find the max
   
  for (i=0;i<MMA8452Q_BUFSZ;i++)
    {
      if (mma8452Q_micros(i)>maxTime)
	{
	  maxTime=mma8452Q_micros(i);
	}
    }
    for (i=0;i<MMA8452Q_BUFSZ;i++)
      {
      if ((mma8452Q_micros(i)+mma8452Q_timeWindow) >= maxTime)        
	{
          nSum++;
          sumX=sumX+mma8452Q_bufX(i);
          sumY=sumY+mma8452Q_bufY(i);
	  sumZ=sumZ+mma8452Q_bufZ(i);
	}
      } 
    aveX=sumX/nSum;
    aveY=sumY/nSum;
    aveZ=sumZ/nSum;
    
    if ( sendAngle || sendGrad) 
      {
	aveX=sumX/nSum;
	aveY=sumY/nSum;
	aveZ=sumZ/nSum;
	mag=sqrt(aveX * aveX + aveY*aveY +aveZ*aveZ);
	slope=sqrt(aveX*aveX + aveY*aveY)/mag;
	if (sendAngle)
	  {
	    int *what=CANbuf+4;
	    float *angle=CANbuf
	    *angle=acos(aveZ/mag);
            *what=4;
            CAN.sendMsgBuf(canId,FRC_EXT,CANlen,CANbuf);
	  }
        if (sendGrad)
          { 
	    int *what=CANbuf+4;
	    float *grad=CANbuf;
            *what=5;
            *grad=slope;
	    CAN.sendMsgBuf(canId,FRC_EXT,CANlen,CANbuf);
	  }
      }
    if (sendXYZ)
      {
	int *what=CANbuf+4;
	float *val=CANbuf;
        *what=1;
        *val=aveX;
	CAN.sendMsgBuf(canID,FRC_EXT,CANlen,CANbuf);
	*what=2;
	*val=aveY;
	CAN.sendMsgBuf(canID,FRC_EXT,CANlen,CANbuf);
        *what=3;
	*val=aveZ;
	CAN.sendMsgBuf(canID,FRC_EXT,CANlen,CANbuf);
      } 
  return(ret);
}


