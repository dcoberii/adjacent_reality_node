
#ifndef L3G4200D_H
#define L3G4200D_H

//I2C address is wither D0 or D2 depending on SA0 
#define L3G4200D_ADDRESS		0xD2

void l3g4200d_init(void);
unsigned char l3g4200d_drdy(void);
void l3g4200d_read(unsigned short *,unsigned short *,unsigned short *);

// Register Address Map
#define L3G4200D_CTRL_REG1              0x20
#define L3G4200D_CTRL_REG2              0x21
#define L3G4200D_CTRL_REG3              0x22
#define L3G4200D_CTRL_REG4              0x23
#define L3G4200D_CTRL_REG5              0x24
#define L3G4200D_STATUS_REG             0x27
#define L3G4200D_OUT_X_L                0x28	

// The register addresses are all 0x7F masked
// For multibyte read/write the address can be set to auto increment via the MSB
#define L3G4200D_AUTO_INCREMENT         0x80

//CTRL_REG1: Data rates
#define L3G4200D_DR_100HZ		0x00
#define L3G4200D_DR_200HZ		0x40
#define L3G4200D_DR_400HZ		0x80
#define L3G4200D_DR_800HZ		0xC0
//CTRL_REG1: Enables
#define L3G4200D_PD			0x08
#define L3G4200D_ZEN			0x04
#define L3G4200D_YEN 			0x02
#define L3G4200D_XEN			0x01
//CTRL_REG3: Interrupts	
#define L3G4200D_H_LACTIVE 		0x20
#define L3G4200D_I2_DRDY		0x08
//CTRL_REG4: Scale
#define L3G4200D_FS_250DPS		0x00
#define L3G4200D_FS_500DPS		0x10
#define L3G4200D_FS_2000DPS		0x30
//STATUS_REG
#define L3G4200D_ZYXDA			0x08
#define L3G4200D_ZYXOR			0x80

#endif
