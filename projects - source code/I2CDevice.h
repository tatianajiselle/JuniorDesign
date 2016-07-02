#ifndef I2C_H_
#define I2C_H_

#define BBB_I2C_0 "/dev/i2c-0"
#define BBB_I2C_1 "/dev/i2c-1"
#define BBB_I2C_2 "/dev/i2c-2"

class I2CDevice{
public:
	I2CDevice(unsigned int _bus, unsigned int _device);
	int open();
	int write(unsigned char value);
	unsigned char readRegister(unsigned int registerAddress);
	unsigned char* readRegisters(unsigned int number, unsigned int fromAddress = 0);
	int writeRegister(unsigned int registerAddress, unsigned char value);
	void debugDumpRegisters(unsigned int number = 0xff);
	void close();
	~I2CDevice();

	//functions for Junior Lab Project
	char* getVoltage();
	void getAnalogVoltage(int AIN);
	void sendByteToLCD(int en, int rs, int value);
	void sendStringToLCD(int en, int rs, char* string);

private:
	unsigned int bus;    //the bus number
	unsigned int device; //the device number on the bus
	int file;            //the file handle to the device
	
	//variables for Junior Design Project
	char analogVoltage[4];
};
#endif /* I2C_H_ */
