//////////////////////////////////////////////////////////////
// File open function
//////////////////////////////////////////////////////////////

static inline bool openByIndex(uint16_t index){
	Serial.println("open by index");
	if(fileIndex[index] == 0) return false;
	if(!file.open(root, fileIndex[index] - 1)) error("File Open Error");
	if(!wave.create(file)) error("Not a valid WAV");
	wavePeriod = waveSize[index] >> 3;
	grainPeriod = wavePeriod >> 4;
	Serial.println(grainPeriod);
	playingFileIndexNo = index;
	return true;
}

//////////////////////////////////////////////////////////////
// Utillity functions from WaveHC library example
//////////////////////////////////////////////////////////////

/*
* print error message and halt
*/
void error_P(const char *str) {
	PgmPrint("Error: ");
	SerialPrint_P(str);
	sdErrorCheck();
	while(1);
}

/*
* print error message and halt if SD I/O error, great for debugging!
*/
void sdErrorCheck(void) {
	if(!card.errorCode()) return;
	PgmPrint("\r\nSD I/O error: ");
	Serial.print(card.errorCode(), HEX);
	PgmPrint(", ");
	Serial.println(card.errorData(), HEX);
	while(1);
}