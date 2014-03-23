// This includes are need to compile on Stino
#include <Wire.h>
#include <MCP23017.h>

// Include libraries
#include <WaveHC_Dm9.h>
#include <WaveUtil_Dm9.h>
#include "UI.h"

#define error(msg) error_P(PSTR(msg))
#define FILE_COUNT 8

#define checkMode(no) ((modeStatus >> no) & 0x01)

#define SHIFT_A 0
#define SHIFT_B 1

#define MODE_LOOP		   0
#define MODE_ONE_SHOT 	   2
#define MODE_PITCH_LOCK    3

#define MODE_MASK_BYTE B00001101

SdReader card;
FatVolume vol;
FatReader root;
FatReader file;
WaveHC wave;

uint16_t fileIndex[FILE_COUNT];
uint32_t waveSize[FILE_COUNT];

uint16_t playingFileIndexNo;
uint32_t playingFilePos;

uint32_t wavePeriod;

uint32_t loopStartPoint;
uint32_t loopEndPoint;

uint8_t looping;
uint8_t lastPressedSW;

uint8_t grainLoopFlag;
uint32_t grainLoopStartPos;
uint32_t grainLoopSize;
uint8_t  grainLoopOverflowFlag;
uint32_t grainPeriod;

uint8_t modeStatus;

UI ui;

void selectWave(){
	if(wave.isplaying) wave.stop();
	ui.setShiftLED(SHIFT_A,1);
	for(int i=0;i<8;i++){
		if(i==playingFileIndexNo) ui.setLEDBlinkRaw(0x04 << i);
		if(fileIndex[i] != 0) ui.setKeyLED(i,HIGH);
		else ui.setKeyLED(i,LOW);
	}

	while(true){
		ui.updateInput();
		for(int i=0;i<8;i++){
			if(ui.justPressedKeySW(i)){
				if(openByIndex(i)) goto WAVE_SELECT_DONE;
			}
		}
		if(ui.justPressedShiftSW(SHIFT_A)) goto WAVE_SELECT_DONE;
		if(ui.justPressedShiftSW(SHIFT_B)) modeSetting();
		ui.updateGUI();
	}

	WAVE_SELECT_DONE: // label
	Serial.println();
	ui.setKeyLEDRaw(0);
	ui.setShiftLED(SHIFT_A,LOW);
	ui.setLEDBlinkRaw(0);
	Serial.print("Loaded File Index : ");
	Serial.println(playingFileIndexNo,DEC);
	ui.updateInput(); // reset sw status
}

void modeSetting(){
	ui.setShiftLED(SHIFT_B,1);
	ui.setKeyLEDRaw(B00001101);
	ui.setLEDBlinkRaw(modeStatus<<2);
	//ui.updateInput();
	while(true){
		ui.updateInput();
		modeToggle(MODE_LOOP);
		modeToggle(MODE_ONE_SHOT);
		modeToggle(MODE_PITCH_LOCK);
		ui.updateGUI();
		if(ui.justReleasedShiftSW(SHIFT_B)) break;
	}

	// Reset LEDs to wave select
	ui.setShiftLED(SHIFT_B,0);
	for(int i=0;i<8;i++){
		if(i==playingFileIndexNo) ui.setLEDBlinkRaw(0x04 << i);
		if(fileIndex[i] != NULL) ui.setKeyLED(i,HIGH);
		else ui.setKeyLED(i,LOW);
	}
}

void modeToggle(uint8_t no){
	if(ui.justPressedKeySW(no)){
		modeStatus ^= 0x01 << no;
		ui.setLEDBlinkRaw(modeStatus<<2);
	}
}

static inline void loopMasherMode(){

	//if playing position is file end, seek to 0
	if(!wave.isplaying && looping != 0){
		if(ui.getShiftSW(SHIFT_B)) grainLoopOverflowFlag = 1;
		wave.seek(0);
		wave.play();
	}

	//grain loop in
	if(ui.justPressedShiftSW(SHIFT_B)){
		ui.setShiftLED(SHIFT_B,HIGH);
		grainLoopOverflowFlag = 0;
		grainLoopStartPos = wave.getPos();
	}

	// grain loop jump to loop start pos
	if(wave.isplaying && ui.getShiftSW(SHIFT_B)) {
		if(wave.getPos() + 1024 >  grainLoopSize * grainPeriod + grainLoopStartPos){
			wave.seek(grainLoopStartPos);
		}
		if(grainLoopOverflowFlag==1 && wave.getPos() + 1024 > grainLoopSize * grainPeriod + grainLoopStartPos - waveSize[playingFileIndexNo]){
			grainLoopOverflowFlag = 0;
			wave.seek(grainLoopStartPos);
		}
	}

	// grain loop out
	if(ui.justReleasedShiftSW(SHIFT_B)){
		ui.setShiftLED(SHIFT_B,LOW);
		grainLoopOverflowFlag = 0;
	}


	// position key pressed
	for(int i=0;i<8;i++){
		if(ui.justPressedKeySW(i)){
			wave.seek(wavePeriod*i);
			if(!wave.isplaying) wave.play();
			looping = 1;
			lastPressedSW = i;
			
			// if in grain loop
			if(ui.getShiftSW(SHIFT_B)){
				grainLoopStartPos = wavePeriod*i;
				grainLoopOverflowFlag = 0;
			}
		}
	}

	// set position LED
	if(wave.isplaying){
		for(int i=1;i<9;i++){
			if(wave.getPos() + 512< wavePeriod*i){
				ui.setKeyLEDRaw(0x01 << (i-1));
				break;
			}
		}
	}
}

static inline void sampleKeyboardMode(){
	for(int i=0;i<8;i++){
		if(!wave.isplaying && ui.getKeySW(lastPressedSW) && checkMode(MODE_ONE_SHOT) == 0){
			wave.seek(0);
			wave.play();
		}

		if(ui.justPressedKeySW(i)){
			wave.seek(wavePeriod*i);
			if(!wave.isplaying) wave.play();
			lastPressedSW = i;
		}

		if(ui.justReleasedKeySW(i)){
			if(i==lastPressedSW){
				wave.stop();
				ui.setKeyLEDRaw(0x00);
				sdErrorCheck();
			}
		}
	}

	if(wave.isplaying){
		if(checkMode(MODE_ONE_SHOT) == 1 && (wave.getPos() + 1024) > wavePeriod*(lastPressedSW + 1)){
			ui.setKeyLEDRaw(0x00);
			wave.stop();
		}
		// set position LED
		for(int i=1;i<9;i++){
			if((wave.getPos() + 1024) <= wavePeriod*i){
				ui.setKeyLEDRaw(0x01 << (i-1));
				break;
			}
		}
	}
}


void setup() {
	ui.init();
	Serial.begin(19200);
	Serial.println("start");
	putstring("Free RAM: ");
	Serial.println(FreeRam()); 

	// SDCard initialize
	if (!card.init()) error("card.init");
	// enable optimized read - some cards may timeout
	card.partialBlockRead(true);
	if (!vol.init(card)) error("vol.init");
	if (!root.openRoot(vol)) error("openRoot");

	// Open files and read file name
	dir_t fileBuf;
	while(root.readDir(fileBuf) > 0){
		// check reading filename is nnnnnnnn.WAV
		if (strncmp_P((char *)&fileBuf.name[8], PSTR("WAV"), 3)) continue;
		Serial.print((char*)fileBuf.name);
		if(fileBuf.name[0] >= '1' && fileBuf.name[0] <= '8'){
			Serial.print(" : File Name OK! ");
			uint8_t indexNo = fileBuf.name[0] - '1';
			if(fileIndex[indexNo] == 0){
				// store fileindex + 1 value
				fileIndex[indexNo] = root.readPosition()/32 - 1 + 1;
				if(!file.open(root, fileIndex[indexNo] - 1)) error("File Open Error");
				//check this is proper wav file
				if (!wave.create(file)) {
					Serial.println(" Not a valid WAV");
					fileIndex[indexNo] = 0;
				}else{
					waveSize[indexNo] = wave.getSize();
					Serial.print(fileIndex[indexNo] - 1,DEC);
					Serial.print(" / ");
					Serial.println(waveSize[indexNo],DEC);		
				}

			}else{
				Serial.print("already indexed at ");
			}
			Serial.println(fileBuf.name[0] - '1',DEC);
		}else{
			Serial.println(" : File Name Wrong");
		}
	}

	int i=0;
	while(i<8){
		if(openByIndex(i) == true) break;
		i++;
	}
	selectWave();
	looping = 0;
	grainLoopFlag = 0;
}

void loop() {
	ui.updateInput();
	if(ui.justPressedShiftSW(SHIFT_A)){
		ui.setKeyLEDRaw(0x00);
		looping = 0;
		selectWave();
	}else{
		if(checkMode(MODE_LOOP)) {
			sampleKeyboardMode();
		}else{
			loopMasherMode();
		}
	}

	if(ui.potChanged(1) && checkMode(MODE_PITCH_LOCK) == 0){
		wave.setSampleRate(wave.dwSamplesPerSec*ui.getPotValue(1)>>7);
	}
	
	if(ui.potChanged(0)){
		grainLoopSize = 63 - (ui.getPotValue(0) >> 2);
		Serial.println(grainLoopSize);
	}

	sdErrorCheck();
	ui.updateGUI();
}