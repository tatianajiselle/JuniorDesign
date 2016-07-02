#include "I2CDevice.h"
#include <cstdlib>
#include <fcntl.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <unistd.h>
using namespace std;

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)
#define CLEAR 0x01
#define FUNCTIONSET 0x38
#define DISPLAYON 0x0E
#define ENTRYMODESET 0x06

I2CDevice::I2CDevice(unsigned int _bus, unsigned int _device) {
	bus = _bus;
	device = _device;
	this->open();
}

int I2CDevice::open() {
   	string name;
   	if(this->bus == 0) name = BBB_I2C_0;
   	else if(this->bus == 1) name = BBB_I2C_1;
   	else name = BBB_I2C_2;

   	if((this->file=::open(name.c_str(), O_RDWR)) < 0) {
   		perror("I2C: failed to open the bus\n");
		return 1;
   	}
   	if(ioctl(this->file, I2C_SLAVE, this->device) < 0) {
     	perror("I2C: Failed to connect to the device\n");
	 	return 1;
   	}
   	return 0;
}

int I2CDevice::writeRegister(unsigned int registerAddress, unsigned char value) {
	unsigned char buffer[2];
   	buffer[0] = registerAddress;
   	buffer[1] = value;
   	if(::write(this->file, buffer, 2) != 2){
    	perror("I2C: Failed write to the device\n");
    	return 1;
   	}
   	return 0;
}

int I2CDevice::write(unsigned char value) {
	unsigned char buffer[1];
   	buffer[0] = value;
   	if (::write(this->file, buffer, 1) != 1){
    	perror("I2C: Failed to write to the device\n");
      	return 1;
   	}
   	return 0;
}

unsigned char I2CDevice::readRegister(unsigned int registerAddress) {
   	this->write(registerAddress);
   	unsigned char buffer[1];
   	if(::read(this->file, buffer, 1) != 1){
      	perror("I2C: Failed to read in the value.\n");
      	return 1;
   	}
   	return buffer[0];
}

unsigned char* I2CDevice::readRegisters(unsigned int number, unsigned int fromAddress) {
	this->write(fromAddress);
	unsigned char* data = new unsigned char[number];
    if(::read(this->file, data, number) != (int)number) {
       	perror("IC2: Failed to read in the full buffer.\n");
	   	return NULL;
    }
	return data;
}

void I2CDevice::debugDumpRegisters(unsigned int number) {
	cout << "Dumping Registers for Debug Purposes:" << endl;
	unsigned char *registers = this->readRegisters(number);
	for(int i = 0; i < (int)number; i++){
		cout << HEX(*(registers + i)) << " ";
		if (i % 16 == 15) cout << endl;
	}
	cout << dec;
}

void I2CDevice::close() {
	::close(this->file);
	this->file = -1;
}

I2CDevice::~I2CDevice() {
	if(file != -1) this->close();
}

char* I2CDevice::getVoltage() {
	return analogVoltage;
}

void I2CDevice::getAnalogVoltage(int AIN) {
	char directory[40];
	sprintf(directory, "/sys/devices/ocp.3/helper.12/AIN%d", AIN);
	std::fstream file(directory,std::ifstream::in);
	if(!file.good()) {
		cout << "LCD::getAnalogVoltage(int): File did not open" << endl;
	  	exit(1);
	}
	file >> this->analogVoltage;
	file.close();
}

void I2CDevice::sendByteToLCD(int en, int rs, int value) {
	this->writeRegister(0x12, (en + rs));
	this->writeRegister(0x13, value);
	this->writeRegister(0x12, rs);
	usleep(5000);
}

void I2CDevice::sendStringToLCD(int en,int rs, char* string) {
	for(int i = 0; string[i] != '\0'; i++) {
		this->writeRegister(0x12, (en + rs));
		this->writeRegister(0x13, string[i]);
		this->writeRegister(0x12, rs);
		usleep(5000);
	}
}

#ifdef MAIN_CPP
int main() {
	double AINVoltage, temperature;
	char *pEnd, tempV[6], tempT[6];
	char *fileString = "/home/debian/Desktop/fire.txt";

	//bus number is 2, device number is 0x20
	I2CDevice *i2c = new I2CDevice(2, 0x20);

	//set GPIO A & B as outputs
	i2c->writeRegister(0x00, 0x00);
	i2c->writeRegister(0x01, 0x00);

	//initialize LCD display on Altera board
	i2c->sendByteToLCD(0x80, 0x00, CLEAR);
	i2c->sendByteToLCD(0x80, 0x00, FUNCTIONSET);
	i2c->sendByteToLCD(0x80, 0x00, DISPLAYON);
	i2c->sendByteToLCD(0x80, 0x00, ENTRYMODESET);
	
	for(;;) {
		//convert voltage from AIN0 pin to temperature value
		i2c->getAnalogVoltage(0);
		AINVoltage = (double)strtol(i2c->getVoltage(), &pEnd, 10) / 1000;
		temperature = -76.699 * AINVoltage + 188.35;
		
		//save temperature value to fire.txt file 
		std::fstream file(fileString, std::ofstream::out);
		if(!file.good()) {
			cout << "File \"fire.txt\" did not open" << endl;
		  	exit(1);
		}
		file << temperature;
		file.close();

		//LCD - display status on top line
		i2c->sendByteToLCD(0x80, 0x00, CLEAR);

		if(AINVoltage > 1.375) 
			i2c->sendStringToLCD(0x80, 0x40, "Normal");
		else if(AINVoltage > 1 && AINVoltage <= 1.375) 
			i2c->sendStringToLCD(0x80, 0x40, "Warm");
		else if(AINVoltage > 0.6875 && AINVoltage <= 1) 
			i2c->sendStringToLCD(0x80, 0x40, "Dangerous");
		else //AINVoltage < 0.6875
			i2c->sendStringToLCD(0x80, 0x40, "Fire! Fire!");
		i2c->sendByteToLCD(0x80, 0x00, 0xC0);

		//LCD - display voltage on bottom line
		sprintf(tempV, "%0.2lf", AINVoltage);
		i2c->sendStringToLCD(0x80, 0x40, tempV);
		i2c->sendByteToLCD(0x80, 0x40, 0x56); //V
		i2c->sendByteToLCD(0x80, 0x40, 0x20); //[SPACE]
		i2c->sendByteToLCD(0x80, 0x40, 0x20); //[SPACE]

		//LCD - display temperature on botton line
		sprintf(tempT, "%0.1lf", temperature);
		i2c->sendStringToLCD(0x80, 0x40, tempT);
		i2c->sendByteToLCD(0x80, 0x40, 0xDF); //[Degree symbol]
		i2c->sendByteToLCD(0x80, 0x40, 0x46); //F
		
		//command to send file data to website
		system("curl --data-urlencode \"file=`cat fire.txt`\" http://107.170.133.224:3000/log");
		usleep(500000);
	}
}
#endif
