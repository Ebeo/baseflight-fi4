// ************************************************************************************************************
// I2C Barometer MS561101BA
// ************************************************************************************************************
// first contribution from Fabio
// modification from Alex (September 2011)
//
// specs are here: http://www.meas-spec.com/downloads/MS5611-01BA03.pdf
// useful info on pages 7 -> 12

// MultiWII 2.0

#include "board.h"

#define MS561101BA_ADDRESS 0x77 //CBR=0 0xEE I2C address when pin CSB is connected to LOW (GND)
//#define MS561101BA_ADDRESS 0x76 //CBR=1 0xEC I2C address when pin CSB is connected to HIGH (VCC)

// registers of the device
#define MS561101BA_PRESSURE    0x40
#define MS561101BA_TEMPERATURE 0x50
#define MS561101BA_RESET       0x1E

// OSR (Over Sampling Ratio) constants
#define MS561101BA_OSR_256  0x00
#define MS561101BA_OSR_512  0x02
#define MS561101BA_OSR_1024 0x04
#define MS561101BA_OSR_2048 0x06
#define MS561101BA_OSR_4096 0x08

#define OSR MS561101BA_OSR_4096

static struct {
  // sensor registers from the MS561101BA datasheet
  uint16_t c[7];
  uint32_t ut; //uncompensated T
  uint32_t up; //uncompensated P

} ms5611ctx;

bool ms5611reset(){
	
	delay(10); 
	
	if (!i2cWrite(MS561101BA_ADDRESS, MS561101BA_RESET, 0))
	{
		delay(10);
		if (!i2cWrite(MS561101BA_ADDRESS, MS561101BA_RESET, 0))
			return false;
	}
	return true;
}

bool ms5611ReadCalibration(){
	int i;
  uint8_t data[2];

	delay(100);

  for(i=0; i<6; i++) {	  
		if (!i2cRead(MS561101BA_ADDRESS, 0xA2+2*i, 2, data))
			return false;
	  ms5611ctx.c[i+1] = (data[0] << 8) | data[1];
  }	
	return true;
}

bool ms5611Init() {
 	
	if (!ms5611reset())
			return false;
	
	return ms5611ReadCalibration();
}

// read uncompensated temperature value: send command first
void ms5611UT_Start() {
  i2cWrite(MS561101BA_ADDRESS, MS561101BA_TEMPERATURE + OSR, 0);
}

// read uncompensated pressure value: send command first
void ms5611UP_Start () {
  i2cWrite(MS561101BA_ADDRESS, MS561101BA_PRESSURE + OSR, 0);
}

// read uncompensated pressure value: read result bytes
void ms5611UP_Read () {
  uint8_t buff[3];
  i2cRead(MS561101BA_ADDRESS, 0, 3, buff);
  ms5611ctx.up = ((uint32_t)buff[0] * 65536) | ((uint32_t)buff[1] * 256) | buff[2];
}

// read uncompensated temperature value: read result bytes
void ms5611UT_Read() {
  uint8_t buff[3];
  i2cRead(MS561101BA_ADDRESS, 0, 3, buff);
	ms5611ctx.ut = ((uint32_t)buff[0] * 65536) | ((uint32_t)buff[1] * 256) | buff[2];
}

int32_t ms5611Calculate() {
	  int32_t temperature,off2=0,sens2=0,delt;

	  int32_t dT   = ms5611ctx.ut - ((uint32_t)ms5611ctx.c[5] << 8);
	  int64_t off  = ((uint32_t)ms5611ctx.c[2] <<16) + (((int64_t)dT * ms5611ctx.c[4]) >> 7);
	  int64_t sens = ((uint32_t)ms5611ctx.c[1] <<15) + (((int64_t)dT * ms5611ctx.c[3]) >> 8);
	  temperature  = 2000 + (((int64_t)dT * ms5611ctx.c[6])>>23);

	  if (temperature < 2000) { // temperature lower than 20st.C
	    delt = temperature-2000;
	    delt  = delt*delt;
	    off2  = (5 * delt)>>1;
	    sens2 = (5 * delt)>>2;
	    if (temperature < -1500) { // temperature lower than -15st.C
	      delt  = temperature+1500;
	      delt  = delt*delt;
	      off2  += 7 * delt;
	      sens2 += (11 * delt)>>1;
	    }
	  }
	  off  -= off2;
	  sens -= sens2;
	  return (( (ms5611ctx.up * sens ) >> 21) - off) >> 15;
}



