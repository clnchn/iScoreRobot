const int width = 30;//vertical distance between notes
const int widthScan = 210; //total distance scanning
const int blockLength = 335; //length the paper needs to be pushed
const int MAXNOTES = 54; // Max notes

void scanColumn (int *height, int *color){
	nMotorEncoder[motorA] = 0;
	motor[motorA] = -3;
	while ((SensorValue[S3]==6 || SensorValue[S3] ==1 )&& nMotorEncoder[motorA] >-75)
	{displayString(1, "%d", SensorValue[S3]);}
	wait1Msec(300);
	motor[motorA] = 0;
	*height = nMotorEncoder[motorA];
	*color = SensorValue[S3];
	displayString (6, "%d %d", *height, *color);
	motor[motorA] = 10;
	while (SensorValue[S4] ==0){}
	motor[motorA] = 0;
}
void resetScanner (){
	motor[motorB] = -10;
	while (SensorValue[S2]==0)
	{}
	nMotorEncoder[motorB]=0;
	motor[motorB] = 0;
}


void movePaper(){
	nMotorEncoder[motorC] = 0;
	motor[motorC] = -30;
	while (nMotorEncoder[motorC] > -blockLength)
	{}
	motor[motorC] = 0;
}



void moveColumn(){
	if (nMotorEncoder[motorB]>= widthScan) {
		resetScanner();
		movePaper();
		nMotorEncoder[motorB]=0;
	}
	else{
		motor[motorB] = 5;
		wait1Msec(500);
		while (nMotorEncoder[motorB]%width !=0)
		{}
		motor[motorB] = 0;
	}
	displayString (3, "%d",nMotorEncoder[motorB]);
}


int readNotes (int *height, int *color){
	int count = 0;
	motor[motorA] = 5;
	while(SensorValue[S4] ==0){}
	nMotorEncoder[motorA] =0;
	nMotorEncoder[motorB] = 0;
	do{ // Possible end conditions: no color read, a specific color read, a specific height read
		scanColumn(&height[count],&color[count]);
		moveColumn();
		count ++;
	}while (color[count-1] != 0);
	return count;
}

void buttonPressed (){
	while(nNxtButtonPressed==-1)
	{}
	while (nNxtButtonPressed!=-1)
	{}
}

int keySignatureReader(int sigNote, int sharpFlat, int keyChanger) //TESTED
{
	int keyChange [7] = {0,0,0,0,0,0,0};
	sigNote = 11-sigNote;
	int note = (int)round(1.7857*(sigNote) - 1.5714) + sharpFlat; //Approximation is accurate for our purposes
	if (note == 7) //Key of C major
	{}
	else if ((note != 0 && note != 10) && note%2 == 0||((note == 1 || note == 9) || note == 11)) //For sharps
	{
		while (note%12 != 7)
		{
			if (note%12 == 8 || note%12 == 1)
			{
				keyChange[(int)round(0.5573*((note%12)-1) + 0.8949)-1] = 1; //Approximation is accurate for our purposes
			}
			else
			{
				keyChange[(int)round(0.5573*(note%12) + 0.8949)-1] = 1;
			}
			note = note+5;
		}
		int placeholder;
		placeholder = keyChange[0];
		for (int count = 0; count < 6; count ++) //THIS IS A TEMPORARY FIX
			keyChange[count] = keyChange[count+1];
	keyChange[6] = placeholder;
	}
	else //For flats
	{
		while (note%12 != 7)
		{
			keyChange[(int)round(0.5573*((note+6)%12) + 0.8949)-1] = -1;
			note = note + 7;
		}
	}
	return keyChange[keyChanger];
}

void readKeySig (int *frequency, int sigNote, int sharpFlat){ //sigNote is height[0], sharpFlat is color[0]
	int keyChange [7];
	for (int i = 0; i< 7; i++){
	keyChange[i] = keySignatureReader(sigNote, sharpFlat, i);
	displayString (i, "%d", keyChange[i]);
}
	for (int  i = 0; i<15; i++){
		frequency[i] *= pow(1.059463,keyChange[6-((i+3)%7)]);
	}
}

int clapToBeats (int timeSig){
	wait1Msec (1000); //sound sensor bounces at first
	int count = 0;
	int beats = 0; //Milliseconds per beats
	while (SensorValue[S1]<60)
	{}
	wait1Msec(250);
	time1[T1] = 0;
	while (count <timeSig-1){
		if (SensorValue[S1]>= 45)
		{
			count++;
			beats += time1[T1];
			time1[T1] = 0;
			wait1Msec(250);
		}
		displayString (0, "%d", count);
	}
	return beats*1.15/count; //Multiplied by 1.15 to factor in error from sound sensor, bpm is 60000/return value
}

int color_time (int color, int beats){
	if (color == 2) //blue 2
		return beats;
	else if (color == 3) //green 1
 		return beats*0.5;
	else if (color == 4) // yellow 4
		return beats*0.25;
	else if (color == 5) // red
		return beats*2;
	return 0;
}

int heightToNote (int height)
{
	const int starterPoint = -10;
	const int notewidth = 5;
	height = height - starterPoint;
	return ((-height)-(-height)%notewidth)/notewidth;
}

void playNotes (int *frequency, int *height, int *color, int timeSig, int multi, int count){
	int beats = clapToBeats(timeSig);
	for (int  i = 2; i< count; i++){
		int time = color_time(color[i],beats)*multi/5;
		playImmediateTone (frequency[heightToNote(height[i])], time);
		wait1Msec(time);
		clearSounds();
		wait1Msec(time/4);
	}
}

task main (){
	SensorType[ S1] = sensorSoundDB;
	SensorType[S2] = sensorTouch;
	SensorType[S3] = sensorColorNxtFULL;
	SensorType[S4] = sensorTouch;
wait1Msec(1000);
    int height [27] = {10,3,9,9,8,9,6,7,9,9,8,9,5,6,9,9,2,4,6,7,8,3,3,4,6,5,6};
int color [27] = {3,2,3,3,2,2,2,5,3,3,2,2,2,5,3,3,2,2,2,2,2,3,3,2,2,2,5};
    int frequency [15] = {989, 880, 784, 698,659,587,523,494,440,392,349,330,294,262,247}; // The frequency of notes from B3 to B5
   // while(buttonPressed()){}
   // int count = readNotes(height, color);
    int count = 27;
    readKeySig (frequency,heightToNote(height[0]), color[0]-3); //Blue flat, green natural, yellow sharp,  a height of F (10) is F major)
    int timeSigBar = heightToNote(height[1]); //height is number of beats
    int timeSigBeat = color[1]*2; //Blue 4, Yellow 8
    buttonPressed();
    playNotes(frequency, height, color, timeSigBar, timeSigBeat, count);
}
//{3,3,8,7,6,5,4,3,2,1}


task main (){
	SensorType[ S1] = sensorSoundDB;
	SensorType[S2] = sensorTouch;
	SensorType[S3] = sensorColorNxtFULL;
	SensorType[S4] = sensorTouch;
wait1Msec(1000);
    int height[MAXNOTES], color [MAXNOTES];
    int frequency [15] = {989, 880, 784, 698,659,587,523,494,440,392,349,330,294,262,247}; // The frequency of notes from B3 to B5
    buttonPressed();
    int count = readNotes(height, color);
    readKeySig (frequency,heightToNote(height[0]), color[0]-3); //Blue flat, green natural, yellow sharp,  a height of F (10) is F major)
    int timeSigBar = heightToNote(height[1]); //height is number of beats

    int timeSigBeat = color[1]*2; //Blue 4, Yellow 8
    buttonPressed();
    playNotes(frequency, height, color, timeSigBar, timeSigBeat, count);
}



Test arrays
Happy Birthday: 
int height [27] = {6,3,9,9,8,9,6,7,9,9,8,9,5,6,9,9,2,4,6,7,8,3,3,4,6,5,6}
int color [27] = {3,2,3,3,2,2,2,5,3,3,2,2,2,5,3,3,2,2,2,2,2,3,3,2,2,2,5}

A la Claire Fontaine 
int height [43] = {10, 2, 10, 10 ,8, 8, 9, 8, 9, 10, 10, 8, 8, 9,8, 8, 8,9, 10,8, 6, 8, 6, 6, 8, 10, 8, 9, 10, 10, 8, 8, 9, 10, 8, 10, 8, 8, 9, 10, 8, 9, 10}
int color [43] = {3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 2, 2, 3, 3, 3, 3, 3, 3, 2, 3, 3, 3, 3, 2, 2, 3, 3, 3, 4, 4, 3, 3, 2, 3, 4, 4, 3, 3, 2}

Scales
int height [17] = {10, 4, 10, 9, 8, 7, 6, 5, 4, 3, 13, 11, 9, 7, 6, 3, 10}
int color [17] = {3, 2, 2, 3, 3, 3, 3, 3, 3, 5, 3, 3, 3, 3, 2, 2, 5 }

task main (){
	SensorType[ S1] = sensorSoundDB;
	SensorType[S2] = sensorTouch;
	SensorType[S3] = sensorColorNxtFULL;
	SensorType[S4] = sensorTouch;
wait1Msec(1000);
    int height [27] = {3,3,9,9,8,9,6,7,9,9,8,9,5,6,9,9,2,4,6,7,8,3,3,4,6,5,6};
int color [27] = {3,2,3,3,2,2,2,5,3,3,2,2,2,5,3,3,2,2,2,2,2,3,3,2,2,2,5};
    int frequency [15] = {989, 880, 784, 698,659,587,523,494,440,392,349,330,294,262,247}; // The frequency of notes from B3 to B5
   // while(buttonPressed()){}
   // int count = readNotes(height, color);
    readKeySig (frequency,height[0], color[0]-3);
    int timeSigBar = height[1];
    int timeSigBeat = color[1]*2; //Blue 4, Yellow 8
    while(buttonPressed()){}
    playNotes(frequency, height, color, timeSigBar, timeSigBeat, count);
}

