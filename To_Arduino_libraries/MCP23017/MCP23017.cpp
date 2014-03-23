#include <Wire.h>
#include <Arduino.h>
#include "MCP23017.h"

///////////////////////////////////////////////////////////////
// MCP23017 Class Functions
///////////////////////////////////////////////////////////////

// public

void MCP23017::init(uint8_t _addr){
	
	addr = MCP23017_ADDR | _addr;
	Wire.begin();

	//init config
	send(addr, 0, MCP23017_IOCON, 0x00);

	//init port 0
	send(addr, 0, MCP23017_GPIO, 0x00, 0x00);

	//init port dirrection all input
	send(addr, 0, MCP23017_IODIR, 0x00, 0x00);

	//init interupt off
	send(addr, 0, MCP23017_GPINTEN, 0x00, 0x00);

	//clear interrupt
	read(MCP23017_PORTA);
	read(MCP23017_PORTB);
}

void MCP23017::setConfig(uint8_t _value){
	
	send(addr, 0, MCP23017_IOCON, _value);
}

void MCP23017::setDirrection(uint8_t _port, uint8_t _value){
	
	send(addr, _port, MCP23017_IODIR, _value);
}

void MCP23017::setPullUp(uint8_t _port, uint8_t _value){
	
	send(addr, _port, MCP23017_GPPU, _value);
}

void MCP23017::setInputPolarity(uint8_t _port, uint8_t _value){
	
	send(addr, _port, MCP23017_IPOL, _value);
}


void MCP23017::write(uint8_t _port, uint8_t _value){
	
	Wire.beginTransmission(addr);
	Wire.write(MCP23017_GPIO + _port);
	Wire.write(_value);
	Wire.endTransmission();
}

uint8_t MCP23017::read(uint8_t _port){
	
	Wire.beginTransmission(addr);
	Wire.write(MCP23017_GPIO + _port);
	Wire.endTransmission();
	Wire.requestFrom(addr, (uint8_t)1);
	return Wire.read();
}

void MCP23017::setInterruptConfig(uint8_t _port, uint8_t _intcon, uint8_t _defval){
	
	send(addr, _port, MCP23017_INTCON, _intcon);
	send(addr, _port, MCP23017_DEFVAL, _defval);

}

void MCP23017::interrupt(uint8_t _port, uint8_t _value){

	send(addr, _port, MCP23017_GPINTEN, _value);
	//Read from GPIO to clear interrupt flag
	read(_port);
}

uint8_t MCP23017::readIntcap(uint8_t _port){

	Wire.beginTransmission(addr);
	Wire.write(MCP23017_INTCAP + _port);
	Wire.endTransmission();
	Wire.requestFrom(addr, (uint8_t)1);
	return Wire.read();
}


// private

void MCP23017::send(uint8_t _addr, uint8_t _port, uint8_t _reg, 
	uint8_t _value){
	
	Wire.beginTransmission(_addr);
	Wire.write(_reg + _port);
	Wire.write(_value);
	Wire.endTransmission();
}

void MCP23017::send(uint8_t _addr, uint8_t _port, uint8_t _reg,
	uint8_t _value1, uint8_t _value2){
	
	Wire.beginTransmission(_addr);
	Wire.write(_reg + _port);
	Wire.write(_value1);
	Wire.write(_value2);
	Wire.endTransmission();
}