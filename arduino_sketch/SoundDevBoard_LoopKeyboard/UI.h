#ifndef _UI_H_
#define _UI_H_

#define SW_SAMPLING_TIME 10
#define LED_BLINK_TIME 250

// shift switches
#define READ_PORTB PINB & 0x03


#define GET_SW(a,b) (a & (0x1 << b))

#define POT_A 1
#define POT_B 0

#include <Arduino.h>
#include <Wire.h>
#include <MCP23017.h>
#include "Metro_Dm9.h"

class UI{

public:

	void init(){
		// init MCP23017
		io.init();
		io.setConfig(B00000010);
		io.setDirrection(0,0xFF); // PORT A inputs
		io.setDirrection(1,0x00); // PORT B outputs
		io.setPullUp(0, 0xFF);
		io.setInputPolarity(0, 0xFF); //set input Active-High
		// io.setInterruptConfig(0,0,0);
		// io.interrupt(0, 0xFF); // enable interrupt of PORTA

		// init Input Port
		DDRB |= 0x03;
		PORTB |= 0x03;

		// init Output Port
		DDRD &= 0x3F;
		PORTD |= 0x3F;

		//init valiables
		changedSWState = 0;
		prevSWState = (io.read(0) << 2) | (~PINB & 0x03);
		currentSWState = prevSWState;
		LEDState = 0;
		LEDBlinkState = 0;
		blinker = 0;

		// Reset Metro
		metroSW = Metro(SW_SAMPLING_TIME,1);
		metroLED = Metro(LED_BLINK_TIME,1);

		changedPotValue = 0;
		for(int i=0;i<2;i++){
			currentPotValue[i] = analogRead(i) >> 2;
		}
	}

	void updateInput(){
		changedSWState = 0;
		
		if(metroSW.check()==1){
			uint16_t newSWState = (io.read(0) << 2) | (~PINB & 0x03);
			for(int i=0;i<10;i++){
				uint16_t temp = GET_SW(newSWState,i);
				//Serial.println(temp,BIN);
				if((temp != GET_SW(currentSWState,i)) && (temp == GET_SW(prevSWState,i))){
					currentSWState &= ~(0x01 << i);
					currentSWState |= temp;
					changedSWState |= 0x01 << i;
				}
			}
			prevSWState = newSWState;
		}

		uint16_t temp=0;
		changedPotValue = 0;
		for(int pin=0;pin<2;pin++){
			temp = 0;
			delay(1);
			analogRead(pin);
			for(int i=0;i<4;i++){
				temp += analogRead(pin);
			}
			temp = temp >> 4;
			temp = temp==255?255:(currentPotValue[pin] + temp) >> 1;
			if(currentPotValue[pin] != temp) {
				changedPotValue |= 0x01 << pin;
				currentPotValue[pin] = temp;
			}
		}
	}

	uint8_t readKeySW(){
		uint8_t temp = (uint8_t)(currentSWState>>2);
		return temp;
	}

	uint16_t readSWRaw(){
		return currentSWState;
	}

	bool justPressedKeySW(uint8_t _pin){
		return (((currentSWState & changedSWState) >> (9 - _pin)) & 0x01) == 1;
	}

	bool justReleasedKeySW(uint8_t _pin){
		return (((~currentSWState & changedSWState) >> (9 - _pin)) & 0x01)== 1;
	}

	bool justPressedShiftSW(uint8_t _pin){
		return (((currentSWState & changedSWState) >> _pin) & 0x01)== 1;
	}

	bool justReleasedShiftSW(uint8_t _pin){
		return (((~currentSWState & changedSWState) >> _pin) & 0x01)== 1;
	}


	uint8_t getKeySW(uint8_t _pin){
		return (currentSWState >> (9 - _pin)) & 0x01;
	}

	uint8_t getShiftSW(uint8_t _pin){
		return (currentSWState >> _pin) & 0x01;
	}

	uint8_t getPotValue(uint8_t _pin){
		return currentPotValue[_pin];
	}

	bool potChanged(uint8_t _pin){
		return (changedPotValue >> _pin) & 0x01;
	}

	void updateGUI(){
		uint16_t temp = 0;
		
		if(metroLED.check() == 1) blinker = (blinker == 1)?0:1;

		for(int i=0;i<10;i++){
			if(((LEDState >> i) & 0x01) == 1 && ((LEDBlinkState >> i) & 0x01) ==1){
				temp |= blinker << i;
			}else{
				temp |= (LEDState & (0x01 << i));
			}
		}

		io.write(1, (temp >> 2));
		PORTD &= 0x3F;
		PORTD |= temp << 6;
	}

	void setKeyLED(uint8_t _pin, uint8_t _state){
		_pin += 2;
		LEDState &= ~(0x01 << _pin);
		LEDState |= _state << _pin;
	}

	void setKeyLEDRaw(uint8_t _date){
		LEDState &= 0x03;
		LEDState |= _date << 2;
	}

	void setShiftLED(uint8_t _pin, uint8_t _state){
		LEDState &= ~(0x01 << _pin);
		LEDState |= _state << _pin;
	}

	void setLEDRaw(uint16_t _data){
		LEDState = _data;
	}

	void setLEDBlinkRaw(uint16_t _data){
		LEDBlinkState = _data;
	}

private:
	MCP23017 io;

	//Metro
	Metro metroSW;
	Metro metroLED;

	//switch
	uint16_t changedSWState;
	uint16_t prevSWState;
	uint16_t currentSWState;

	//POT
	uint8_t  changedPotValue;
	uint8_t currentPotValue[2];

	//LED
	uint16_t LEDState;
	uint16_t LEDBlinkState;

	uint8_t  blinker;

};

#endif
