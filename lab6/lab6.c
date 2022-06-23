#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>
#include <math.h>


#define LEDR_BASE ((volatile unsigned int * ) 0xFF200000)
#define HEX0_3 ((volatile unsigned int * ) 0xFF200020)
#define HEX4_5 ((volatile unsigned int * ) 0xFF200030)
#define LEDS ((volatile unsigned int * ) 0xFF200000)
#define SYSMGR_GENERALIO7 ((volatile unsigned int * ) 0xFFD0849C)
#define SYSMGR_GENERALIO8 ((volatile unsigned int * ) 0xFFD084A0)
#define SYSMGR_I2C0USEFPGA ((volatile unsigned int * ) 0xFFD08704)
#define I2C0_ENABLE ((volatile unsigned int * ) 0xFFC0406C)
#define I2C0_ENABLE_STATUS ((volatile unsigned int * ) 0xFFC0409C)
#define I2C0_CON ((volatile unsigned int * ) 0xFFC04000)
#define I2C0_TAR ((volatile unsigned int * ) 0xFFC04004)
#define I2C0_FS_SCL_HCNT ((volatile unsigned int * ) 0xFFC0401C)
#define I2C0_FS_SCL_LCNT ((volatile unsigned int * ) 0xFFC04020)
#define I2C0_DATA_CMD ((volatile unsigned int * ) 0xFFC04010)
#define I2C0_RXFLR ((volatile unsigned int * ) 0xFFC04078)
#define ADXL345_REG_DEVID 0x00
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_POWER_CTL 0x2D
#define ADXL345_REG_BW_RATE 0x2C
#define ADXL345_REG_INT_SOURCE 0x30
#define XL345_RANGE_2G 0x00
#define XL345_FULL_RESOLUTION 0x08
#define XL345_RATE_100 0x0A
#define XL345_STANDBY 0x00
#define XL345_MEASURE 0x08
#define XL345_DATAREADY 0x80
#define XL345_RANGE_16G 0x03
#define XL345_RANGE_2G 0x00


 // Initialize the ADXL345 chip
/* Single byte Write */
static void ADXL345_REG_WRITE(uint8_t address, uint8_t value) {

  *(I2C0_DATA_CMD) = address + 0x400;
  *(I2C0_DATA_CMD) = value;
}
void ADXL345_Init() {

  // +- 16g range, full resolution
  ADXL345_REG_WRITE(ADXL345_REG_DATA_FORMAT, XL345_RANGE_2G | XL345_FULL_RESOLUTION);
  // Output Data Rate: 100Hz
  ADXL345_REG_WRITE(ADXL345_REG_BW_RATE, XL345_RATE_100);
  // stop measure
  ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_STANDBY);
  // start measure
  ADXL345_REG_WRITE(ADXL345_REG_POWER_CTL, XL345_MEASURE);
	
}
// Read value from internal register at address
void ADXL345_REG_READ(uint8_t address, uint8_t * value) {

  // Send reg address (+0x400 to send START signal)
  * I2C0_DATA_CMD = address + 0x400;
  // Send read signal
  * I2C0_DATA_CMD = 0x100;
  // Read the response (first wait until RX buffer contains data)
  while ( * I2C0_RXFLR == 0) {}
  * value = * I2C0_DATA_CMD;
}
// Return true if there is new data
bool ADXL345_IsDataReady() {
  bool bReady = false;
  uint8_t data8;

  ADXL345_REG_READ(ADXL345_REG_INT_SOURCE, & data8);
  if (data8 & XL345_DATAREADY)
    bReady = true;

  return bReady;
}

/* Multiple Byte Write */
void ADXL345_REG_MULTI_READ(uint8_t address, uint8_t values[], uint8_t len) {

  int i = 0;
  int nth_byte = 0;
  *(I2C0_DATA_CMD) = address + 0x400;

  //send read signal multiple times to prevent overwritten data at 
  //inconsistent times

  for (i = 0; i < len; i++)
    *
    (I2C0_DATA_CMD) = 0x100;

  while (len) {
    if ( * (I2C0_RXFLR) > 0) {
      values[nth_byte] = * (I2C0_DATA_CMD) & 0xFF;
      nth_byte++;
      len--;
    }
  }
}
// Read acceleration data of all three axes
void ADXL345_XYZ_Read(int16_t szData16[3]) {

  uint8_t szData8[6];
  ADXL345_REG_MULTI_READ(0x32, (uint8_t * ) & szData8, sizeof(szData8));

  szData16[0] = (szData8[1] << 8) | szData8[0];
  szData16[1] = (szData8[3] << 8) | szData8[2];
  szData16[2] = (szData8[5] << 8) | szData8[4];
}

void I2C0_Init() {

  // Abort any ongoing transmits and disable I2C0.
  * I2C0_ENABLE = 2;

  // Wait until I2C0 is disabled
  while ((( * I2C0_ENABLE_STATUS) & 0x1) == 1) {}
  // Configure the config reg with the desired setting (act as
  // a master, use 7bit addressing, fast mode (400kb/s)).
  * I2C0_CON = 0x65;

  // Set target address (disable special commands, use 7bit addressing)
  * I2C0_TAR = 0x53;

  // Set SCL high/low counts (Assuming default 100MHZ clock input to
  //I2C0 Controller).
  // The minimum SCL high period is 0.6us, and the minimum SCL low
  //period is 1.3 us,
  // However, the combined period must be 2.5us or greater, so add 0.3us
  //to each.
  * I2C0_FS_SCL_HCNT = 60 + 30; // 0.6us + 0.3us
  * I2C0_FS_SCL_LCNT = 130 + 30; // 1.3us + 0.3us

  // Enable the controller
  * I2C0_ENABLE = 1;

  // Wait until controller is powered on
  while ((( * I2C0_ENABLE_STATUS) & 0x1) == 0) {}
}



void Pinmux_Config() {
  * SYSMGR_I2C0USEFPGA = 0;
  * SYSMGR_GENERALIO7 = 1;
  * SYSMGR_GENERALIO8 = 1;
}
void display_function(int16_t szData16[3])
{
	int temp = 0;
	int X = 0;
	int YZ = 0;
 	szData16[0] = szData16[0]*3.9;
	szData16[1] = szData16[1]*3.9;
	szData16[2] = szData16[2]*3.9;
		switch ((abs(szData16[2])-abs(szData16[2]) % 10)/10)
	//switch (1)
	{
		case 0:
			YZ += 0x3F;	
			break;
		case 1:
			YZ += 0x6;	
			break;
		case 2:
			YZ += 0x5B;
			break;
		case 3:
			YZ += 0x4F;	
			break;
		case 4:
			YZ += 0x66;	
			break;
		case 5:
			YZ += 0x6D;	
			break;
		case 6:
			YZ += 0x7D;	
			break;
		case 7:
			YZ += 0x7;	
			break;
		case 8:
			YZ += 0x7F;	
			break;
		case 9:
			YZ += 0x6F;	
			break;	
	}
	 	YZ = YZ<<8;	
	switch (abs(szData16[2]) % 10)
	//switch (1)
	{
		case 0:
			YZ = 0x3F;	
			break;
		case 1:
			YZ = 0x6;	
			break;
		case 2:
			YZ = 0x5B;
			break;
		case 3:
			YZ = 0x4F;	
			break;
		case 4:
			YZ = 0x66;	
			break;
		case 5:
			YZ = 0x6D;	
			break;
		case 6:
			YZ = 0x7D;	
			break;
		case 7:
			YZ = 0x7;	
			break;
		case 8:
			YZ = 0x7F;	
			break;
		case 9:
			YZ = 0x6F;	
			break;
	}
		YZ = YZ<<8;	
	
		switch ((abs(szData16[1])-abs(szData16[1]) % 10)/10)
	//switch (9)
	{
		case 0:
			YZ += 0x3F;	
			break;
		case 1:
			YZ += 0x6;	
			break;
		case 2:
			YZ += 0x5B;
			break;
		case 3:
			YZ += 0x4F;	
			break;
		case 4:
			YZ += 0x66;	
			break;
		case 5:
			YZ += 0x6D;	
			break;
		case 6:
			YZ += 0x7D;	
			break;
		case 7:
			YZ += 0x7;	
			break;
		case 8:
			YZ += 0x7F;	
			break;
		case 9:
			YZ += 0x6F;	
			break;	
	}
		YZ = YZ<<8;	
	switch (abs(szData16[1]) % 10)
	//switch (6)
	{
		case 0:
			YZ += 0x3F;	
			break;
		case 1:
			YZ += 0x6;	
			break;
		case 2:
			YZ += 0x5B;
			break;
		case 3:
			YZ += 0x4F;	
			break;
		case 4:
			YZ += 0x66;	
			break;
		case 5:
			YZ += 0x6D;	
			break;
		case 6:
			YZ += 0x7D;	
			break;
		case 7:
			YZ += 0x7;	
			break;
		case 8:
			YZ += 0x7F;	
			break;
		case 9:
			YZ += 0x6F;	
			break;		
	}
	
		switch ((abs(szData16[0])-abs(szData16[0]) % 10)/10)
//	switch (5)
	{
		case 0:
			X += 0x3F;	
			break;
		case 1:
			X += 0x6;	
			break;
		case 2:
			X += 0x5B;
			break;
		case 3:
			X += 0x4F;	
			break;
		case 4:
			X += 0x66;	
			break;
		case 5:
			X += 0x6D;	
			break;
		case 6:
			X += 0x7D;	
			break;
		case 7:
			X += 0x7;	
			break;
		case 8:
			X += 0x7F;	
			break;
		case 9:
			X += 0x6F;	
			break;	
	}
		X = X<<8;	
	switch (abs(szData16[0]) % 10)
//	switch (8)
	{
		case 0:
			X = 0x3F;	
			break;
		case 1:
			X = 0x6;	
			break;
		case 2:
			X = 0x5B;
			break;
		case 3:
			X = 0x4F;	
			break;
		case 4:
			X = 0x66;	
			break;
		case 5:
			X = 0x6D;	
			break;
		case 6:
			X = 0x7D;	
			break;
		case 7:
			X = 0x7;	
			break;
		case 8:
			X = 0x7F;	
			break;
		case 9:
			X = 0x6F;	
			break;	
	}
	if(szData16[2] < 0)
	{
		temp = temp + 0b00001000;
	}
	if(szData16[1] < 0)
	{
		temp = temp + 0b00100000;
	}
	if(szData16[0] < 0)
	{
		temp = temp + 0b10000000;
	}
	*(LEDR_BASE) = temp;
	*(HEX4_5) = X;
	*(HEX0_3) = YZ;
}

int main(void) {

  uint8_t devid;
  int16_t mg_per_lsb = 10;
  int16_t XYZ[3];

  // Configure Pin Muxing
  Pinmux_Config();

  // Initialize I2C0 Controller
  I2C0_Init();

  // 0xE5 is read from DEVID(0x00) if I2C is functioning correctly
  ADXL345_REG_READ(0x00, & devid);

  // Correct Device ID
  if (devid == 0xE5) {
    // Initialize accelerometer chip
    ADXL345_Init();

    while (1) {
      if (ADXL345_IsDataReady()) {
        ADXL345_XYZ_Read(XYZ);
        printf("X=%d mg, Y=%d mg, Z=%d mg\n", XYZ[0] * mg_per_lsb,
          XYZ[1] * mg_per_lsb, XYZ[2] * mg_per_lsb);
		  display_function(XYZ);
      }
    }
  } else {
    printf("Incorrect device ID\n");
  }

  return 0;
}
