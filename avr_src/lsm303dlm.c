#include "lsm303dlm.h"
#include "twi.h"

void lsm303dlm_init(void){

	unsigned char buf[6];

	//Accelerometer:
	//We will set 5 contiguous ctrl registers located at CTRL_REG1
	buf[0] = LSM303DLM_AUTO_INCREMENT | LSM303DLM_CTRL_REG1_A;
	
	buf[1] = LSM303DLM_DR_400HZ_A |		// Data rate          CTRL_REG1
			 LSM303DLM_PD_A |			// Power on
			 LSM303DLM_ZEN_A |			// Z-Axis on
			 LSM303DLM_YEN_A |			// Y-Axis on 	
			 LSM303DLM_XEN_A;			// X-Axis on
	buf[2] = 0x00;						//                    CTRL_REG2
	buf[3] = LSM303DLM_IHL_A |			// Active low         CTRL_REG3
			 LSM303DLM_I1_DRDY_A;		// Enable DRDY interrupt pin on setting of ZYXDA (new data)
	buf[4] = LSM303DLM_FS_2G_A |		// Scale selection
			 LSM303DLM_BDU_A;			// Block updates during H/L byte read
	buf[5] = 0x00;						// CTRL_REG5
	
	twi_write_block(LSM303DLM_ADDRESS_A, buf, 6);

	//Magnotometer:
	//We will set 3 contiguous ctrl registers located at CRA_REG
	buf[0] = LSM303DLM_AUTO_INCREMENT | LSM303DLM_CRA_REG_M;
	
	buf[1] = LSM303DLM_DR_30HZ_M;		// Data rate        CRA_REG
	buf[2] = LSM303DLM_GN_1_3G_M;		// Gain             CRB_REG  
	buf[3] = LSM303DLM_CONTINUOUS_M;	// MR               MR_REG
	
	twi_write_block(LSM303DLM_ADDRESS_M, buf, 4);

}

//Check for new accelerometer data
unsigned char lsm303dlm_a_drdy(void){
	
	//The data available bit of the status reg acts just like the DRDY interrupt pin (or rather the other way around see LSM303DLM_I1_DRDY_A of LSM303DLM_CTRL_REG3_A)
	//It is set when new data is available, and cleared once the 6 bytes have been read
	return twi_write_then_read_byte(LSM303DLM_ADDRESS_A,LSM303DLM_STATUS_REG_A) & LSM303DLM_ZYXDA_A;
	
}


void lsm303dlm_a_read(unsigned short *x,unsigned short *y,unsigned short *z){

	unsigned char buf[6];
	buf[0] =  LSM303DLM_AUTO_INCREMENT | LSM303DLM_OUT_X_L_A;
	twi_write_then_read_block(LSM303DLM_ADDRESS_A,buf,1,buf, 6);

	*x = ((unsigned short)buf[1] << 8) + buf[0];
	*y = ((unsigned short)buf[3] << 8) + buf[2];
	*z = ((unsigned short)buf[5] << 8) + buf[4];

}

//FIXME: this doesn't work, always returns true
// it feels like the read operation isn't clearing the drdy bit, perhaps the config is banjaxed?
unsigned char lsm303dlm_m_drdy(void){

	return twi_write_then_read_byte(LSM303DLM_ADDRESS_M,LSM303DLM_SR_REG_M) & LSM303DLM_DRDY_M;
  
}

void lsm303dlm_m_read(unsigned short *x,unsigned short *y,unsigned short *z){

	unsigned char buf[6];
	buf[0] =  LSM303DLM_AUTO_INCREMENT | LSM303DLM_OUT_X_H_M;
	twi_write_then_read_block(LSM303DLM_ADDRESS_M,buf,1,buf, 6);

	*x = ((unsigned short)buf[0] << 8) + buf[1];
	*z = ((unsigned short)buf[2] << 8) + buf[3];
	*y = ((unsigned short)buf[4] << 8) + buf[5];

}
