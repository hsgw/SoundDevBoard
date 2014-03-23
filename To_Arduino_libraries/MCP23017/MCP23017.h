#ifndef _MCP23017_H_
#define _MCP23017_H_

class MCP23017{
public:
	void init(uint8_t _addr = 0);

	void setConfig(uint8_t _value);
	void setDirrection(uint8_t _port, uint8_t _value);
	void setPullUp(uint8_t _port, uint8_t _value);
	void setInputPolarity(uint8_t _port, uint8_t _value);

	void write(uint8_t _port, uint8_t _value);
	uint8_t read(uint8_t _port);
	void setInterruptConfig(uint8_t _port, uint8_t _intcon, uint8_t _defval);
	void interrupt(uint8_t _port, uint8_t _value);
	uint8_t readIntcap(uint8_t _port);
private:
	uint8_t addr;

	void send(uint8_t _addr, uint8_t _port, uint8_t _reg, uint8_t _value);
	void send(uint8_t _addr, uint8_t _port, uint8_t _reg, uint8_t _value1, uint8_t _value2);
};

///////////////////////////////////////////////////////////////
///Useful defines
///////////////////////////////////////////////////////////////

#define MCP23017_ADDR 0x20

#define MCP23017_PORTA 0x00
#define MCP23017_PORTB 0x01

///////////////////////////////////////////////////////////////
///Control Register
///////////////////////////////////////////////////////////////

// IO Dirrection
#define MCP23017_IODIR 0x00

// Input polarity
#define MCP23017_IPOL 0x02

// Interrupt-On-Change enable
#define MCP23017_GPINTEN 0x04

// Default intterupt comparison value
// Refer to INTCON
#define MCP23017_DEFVAL 0x06

// Intterupt compare value
// 1 = compare against DEFVAL value
// 0 = compare against the previous value
#define MCP23017_INTCON 0x08

// IC setting
// bank/mirror/seqop/disslw/haen/odr/intpol/notimp
// must be set bank = 0 for using this library
// See MCP23017 datasheet
#define MCP23017_IOCON 0x0A

// Pullup
#define MCP23017_GPPU 0x0C

// Interrupt Flag
// 1 = pin caused interrupt
// 0 = no interrupt 
// Read-Only
#define MCP23017_INTF 0x0E

// Interrupt capture
// value of GPIO at time of last interrupt
// Read-Only
#define MCP23017_INTCAP 0x10

// GPIO
#define MCP23017_GPIO 0x12

// Output Latch
#define MCP23017_OLAT 0x14

#endif