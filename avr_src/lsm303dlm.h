
#ifndef LSM303DLM_H
#define LSM303DLM_H

//Seperate address for each function
#define LSM303DLM_ADDRESS_M		0x3C
//Accelerometer address is either 0x30 or 0x32 depending on SA0
#define LSM303DLM_ADDRESS_A		0x30

void lsm303dlm_init(void);
unsigned char lsm303dlm_a_drdy(void);
void lsm303dlm_a_read(unsigned short *,unsigned short *,unsigned short *);
unsigned char lsm303dlm_m_drdy(void);
void lsm303dlm_m_read(unsigned short *,unsigned short *,unsigned short *);

// Register Address Map
#define LSM303DLM_CTRL_REG1_A		0x20
#define LSM303DLM_CTRL_REG2_A		0x21
#define LSM303DLM_CTRL_REG3_A		0x22
#define LSM303DLM_CTRL_REG4_A		0x23
#define LSM303DLM_CTRL_REG5_A		0x24
#define LSM303DLM_STATUS_REG_A		0x27
#define LSM303DLM_OUT_X_L_A		0x28	

#define LSM303DLM_CRA_REG_M		0x00		
#define LSM303DLM_CRB_REG_M		0x01		
#define LSM303DLM_MR_REG_M		0x02
#define LSM303DLM_OUT_X_H_M		0x03
#define LSM303DLM_SR_REG_M		0x09

// The register addresses are all 0x7F masked
// For multibyte read/write the address can be set to auto increment via the MSB
#define LSM303DLM_AUTO_INCREMENT         0x80

//CTRL_REG1_A: Data rates
#define LSM303DLM_DR_50HZ_A		0x00
#define LSM303DLM_DR_100HZ_A		0x08
#define LSM303DLM_DR_400HZ_A		0x10
#define LSM303DLM_DR_1000HZ_A		0x18
//CTRL_REG1_A: Enables
#define LSM303DLM_PD_A			0x20
#define LSM303DLM_ZEN_A			0x04
#define LSM303DLM_YEN_A 		0x02
#define LSM303DLM_XEN_A			0x01
//CTRL_REG3_A: Interrupts	
#define LSM303DLM_IHL_A			0x80
#define LSM303DLM_I1_DRDY_A		0x02
//CTRL_REG4_A: Scale
#define LSM303DLM_BDU_A			0x80
#define LSM303DLM_FS_2G_A		0x00
#define LSM303DLM_FS_4G_A		0x10
#define LSM303DLM_FS_8G_A		0x30
//STATUS_REG_A:
#define LSM303DLM_ZYXDA_A		0x08
#define LSM303DLM_ZYXOR_A		0x80

//CRA_REG_M:
#define LSM303DLM_DR_0_75HZ_M		0x00
#define LSM303DLM_DR_1_5HZ_M		0x04
#define LSM303DLM_DR_3HZ_M		0x08
#define LSM303DLM_DR_7_5HZ_M		0x0C
#define LSM303DLM_DR_15HZ_M		0x10
#define LSM303DLM_DR_30HZ_M		0x14
#define LSM303DLM_DR_75HZ_M		0x18
#define LSM303DLM_DR_220HZ_M		0x1C
//CRB_REG_M:
#define LSM303DLM_GN_1_3G_M		0x20
#define LSM303DLM_GN_1_9G_M		0x40
#define LSM303DLM_GN_2_5G_M		0x60
#define LSM303DLM_GN_4_0G_M		0x80
#define LSM303DLM_GN_4_7G_M		0xA0
#define LSM303DLM_GN_5_6G_M		0xC0
#define LSM303DLM_GN_8_1G_M		0xE0
//MR_REG_M
#define LSM303DLM_CONTINUOUS_M		0x00
//SR_REG_M:
#define LSM303DLM_DRDY_M		0x01


#endif
