/* ----------------------------------------------------------------------------
   Party Circuits Code for use with the Party Circuits lesson plan 
   available from Microsoft Education Workshop at http://aka.ms/hackingSTEM 
 
   This project uses an Arduino UNO microcontroller board. More information can
   be found by visiting the Arduino website: 
   https://www.arduino.cc/en/main/arduinoBoardUno 
  
  This project involves one or more LEDs connected into the Arduino Digital inputs.
  The lights are controlled via the Excel workbook.
 
  Comments, contributions, suggestions, bug reports, and feature requests 
  are welcome! For source code and bug reports see: 
  http://github.com/[TODO github path to Hacking STEM] 

  Jen Fox, 2018 Microsoft Education Workshop
  For issues with this code visit: https://aka.ms/hackingstemsupport
 
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights to
  use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
  of the Software, and to permit persons to whom the Software is furnished to do
  so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
  SOFTWARE. 
----------------------------------------------------------------------------
*/ 

// Sample Command for Testing
// 0,3EC,2C4,364,154,374,248,3A8,2BC

#include <String.h>
// ----------------------------------------------------------
// Program variables 
// ----------------------------------------------------------
//Total number of LEDs (max. of 6)
int kNumberOfLeds = 6;
//  Create an array to hold the LED pins
int ledArray[] = {3, 5, 6, 9, 10, 11};
//  Duration that LEDs are OFF (in milliseconds)
const int ledTimeOff = 50;

//Declare variables for Excel commands
bool priorLoopValue; 
int intensityRaw;
int intensity;
int flashSpeedRaw;
int flashSpeed;
//   LED status (0 = off, 1 = on)
int ledStatusArray[6] = {};

// Variables for LED intensity and Flash Speed. Can adjust as desired.
int ledBright = 255;
int ledDim = 100;
int fastSpeed = 250;
int medSpeed = 500;
int slowSpeed = 1000;

// Bitwise operator variables 
int bitwiseArray[8][2] =
{{0x300, 8},
{0xC0, 6},
{0x20, 5},
{0x10, 4},
{0x8, 3},
{0x4, 2},
{0x2, 1},
{0x1, 0}};
int ledHexArray [8];

// Excel variables ------------------------------------------------------------
int commandNumber;
int loopTrack = 0; //variable to send to Excel to keep track of loop iterations
float currentCommand = 0; //variable to store current command
float priorCommand; //variable to store prior command

float incomingExcelFloat = 0; // Command Trigger 
          
// Serial data variables ------------------------------------------------------
// IMPORTANT: This must be equal to number of channels set in Data Streamer
const byte kNumberOfChannelsFromExcel = 9; //Incoming Serial Data Array

String ledSettingArray[kNumberOfChannelsFromExcel][10];

String incomingSerialData[kNumberOfChannelsFromExcel];

const String kDelimiter = ",";    // Data Streamer expects a comma delimeter
String inputString = "";          // String variable to hold incoming data
boolean stringComplete = false;   // Indicates complete string (newline found)
const int kSerialInterval = 50;   // Interval between serial writes
unsigned long serialPreviousTime; // Timestamp to track serial interval

// SETUP ----------------------------------------------------------------------
void setup() {
  // Initialize led pins as outputs! 
  for(int i; i < kNumberOfLeds; i++){
    pinMode(ledArray[i], OUTPUT);
  }
 
  //Initialize the serial port
  Serial.begin(9600);
}

// START OF MAIN LOOP --------------------------------------------------------- 
void loop()
{
  // Read Excel variables from serial port (Data Streamer)
  processIncomingSerial();
  delay(1000); // This is just for debugging
  
  for (int i = 0; i < kNumberOfChannelsFromExcel; i++)
  {
    parseHexValues(i+1);
    flashLeds(i);
    processOutgoingSerial();
    // Need to debug? Print the ledHexArray to determine if it matches the Excel commands
    // for (int j = 0; j < 8; j++){
    // Serial.print(ledHexArray[j], DEC);
    // Serial.print(",");
    // }
    // Serial.println();
  }
//More debugging! Uncomment if necessary
  // for (int i = 0; i < 8; i++){
  //   Serial.println(ledHexArray[i], BIN);
  // }

}
//-------------------------------------------------------------------
// Party Circuits Functions
//-------------------------------------------------------------------

void parseHexValues(int index)
{
    char stringCopy[incomingSerialData[index].length()+1];   // Array to chars 
    incomingSerialData[index].toCharArray(stringCopy, incomingSerialData[index].length()+1); // Convert String object to char[]
    int hex = strtol(stringCopy, NULL, 16); // string to long converts str to string w/ any base
    for (int i = 0; i < 8; i++){
      int val = hex & bitwiseArray[i][0]; // filter out all bits not relevant for command
      val = val >> bitwiseArray[i][1]; // shift all bits to get relevant command
      ledHexArray[i] = val;
    }
}

// Excel-driven LED Flashing Function
void flashLeds(int commandNum){
    // Separate each column of incomingSerialData into separate commands
       intensityRaw= ledHexArray[0];
       flashSpeedRaw = ledHexArray[1];
       for(int i = 0; i < kNumberOfLeds; i++){
        ledStatusArray[i] = ledHexArray[i+2];
       }

       //Determine intensity and LED flash speed from Excel Commands
       intensity = ledIntensity(intensityRaw);
       flashSpeed = ledSpeed(flashSpeedRaw);

      //Determine command number based on input 
       commandNumber = commandNum;
      
       // Flash appropriate LEDs at given intensity       
       for(int i = 0; i < kNumberOfLeds; i++){
        if(ledStatusArray[i] == 1){
          analogWrite(ledArray[i], intensity);
        } 
        else(analogWrite(ledArray[i], LOW));
       }
       
       delay(flashSpeed);

       //Turn off all LEDs
       for(int i = 0; i < kNumberOfLeds; i++){
          analogWrite(ledArray[i], LOW);
        } 
       
       delay(ledTimeOff); //same off duration for all sequences
     
  // Keep track of number of times command sequence is repeated
  loopTrack++;
}

// Check if pattern is repeated  (Excel command 0 = infinite loop, 1 = play once
bool isLoop(int loopCommand){
  if(loopCommand == 1){
    return true;
  }
  else{
    return false; 
  } 
}

// Check value of LED intensity
int ledIntensity(int intensity){
  if(intensity == 1){
    return ledDim;
  }
  else if(intensity == 2){
    return ledBright;
  }
  else{
    return 0;
  }
}

// Check and determine speed
int ledSpeed(int flashSpeed){
  if(flashSpeed == 1){
    return slowSpeed;
  }
  else if(flashSpeed == 2){
    return medSpeed;
  }
  else if(flashSpeed == 3){
    return fastSpeed;
  }
  else{
    return fastSpeed;
  }
}

void parseLedSettings()
{
  for (int i = 0; i < kNumberOfChannelsFromExcel; i++){
    char stringCopy[incomingSerialData[i+1].length()+1];
    incomingSerialData[i+1].toCharArray(stringCopy, incomingSerialData[i+1].length()+1);
    char *token = strtok(stringCopy, ",");
    int j = 0;
    while (token != NULL){
      ledSettingArray[i][j] = token;
      token = strtok(NULL, ",");
      j++;
    }
  }
}

//Function for specialized string search: go through a string and pull out characters
String getValue(String dataString, char separator, int index)
{                                           // basic searching algorithm
                                            // data is the serial string, separator is a comma, index is where we want to look in the data array
  int matchingIndex = 0;                    // no match because we are starting to look
  int strIndex[] = {0, -1};
  int maxIndex = dataString.length()-1;
  for(int i=0; i<=maxIndex && matchingIndex<=index; i++){     // loop until end of array or until we find a match
    if(dataString.charAt(i)==separator || i==maxIndex){             // if we hit a comma OR we are at the end of the array
      matchingIndex++;                                        // increment matchingIndex to keep track of where we have looked
      strIndex[0] = strIndex[1]+1;                            // set substring parameters
           // ternary operator in objective c is [condition] ? [true expression] : [false expression] 
      strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }
  return matchingIndex>index ? dataString.substring(strIndex[0], strIndex[1]) : ""; // if match return substring or else return ""
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
// Process serial data inputString from Data Streamer
void ParseSerialData()
{
  if (stringComplete) {     
    //Build an array of values from comma delimited string from Data Streamer
    BuildDataArray(inputString);

    // Set variables based on array index referring to columns:
    // Data Out column A5 = 0, B5 = 1, C5 = 2, etc.
    incomingExcelFloat = incomingSerialData[0].toFloat(); // Data Out column A5
    //incomingExcelString1 = incomingSerialData[1]; // Data Out column B5
       
    inputString = ""; // reset inputString
    stringComplete = false; // reset stringComplete flag
  }

}

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void sendDataToSerial()
{
  // Send data out separated by a comma (kDelimiter)
  //Loop track number
  Serial.print(loopTrack);
  Serial.print(kDelimiter);
  
  // Example test for incoming Excel variables
  Serial.print(incomingExcelFloat);
  Serial.print(kDelimiter);

  //LED status (on/off)
  Serial.print(commandNumber);
  Serial.print(kDelimiter);
  Serial.print(ledHexArray[2]);
  Serial.print(kDelimiter);
  Serial.print(ledHexArray[3]);
  Serial.print(kDelimiter);
  Serial.print(ledHexArray[4]);
  Serial.print(kDelimiter);
  Serial.print(ledHexArray[5]);
  Serial.print(kDelimiter);
    
  Serial.println(); // Add final line ending character only once
}

//-----------------------------------------------------------------------------
// DO NOT EDIT ANYTHING BELOW THIS LINE
//-----------------------------------------------------------------------------

// OUTGOING SERIAL DATA PROCESSING CODE----------------------------------------
void processOutgoingSerial()
{
   // Enter into this only when serial interval has elapsed
  if((millis() - serialPreviousTime) > kSerialInterval) 
  {
    serialPreviousTime = millis(); // Reset serial interval timestamp
    sendDataToSerial(); 
  }
}

// INCOMING SERIAL DATA PROCESSING CODE----------------------------------------
void processIncomingSerial()
{
  getSerialData();
  ParseSerialData();
}

// Gathers bits from serial port to build inputString
void getSerialData(){
  if(Serial.available()){
    inputString = Serial.readStringUntil('\n');
    stringComplete =true;
  }
}

// Takes the comma delimited string from Data Streamer
// and splits the fields into an indexed array
void BuildDataArray(String data)
{
  return ParseLine(data);
}

// Parses a single string of comma delimited values with line ending character
void ParseLine(String data)  
{
    int charIndex = 0; // Tracks the character we are looking at
    int arrayIndex = 0; // Tracks the array index to set values into
    while(arrayIndex < kNumberOfChannelsFromExcel) // Loop until full
    {
        String field = ParseNextField(data, charIndex);  // Parse next field
        incomingSerialData[arrayIndex] = field; // Add field to array
        arrayIndex++;   // Increment index
    }
}

// Parses the next value field in between the comma delimiters
String ParseNextField(String data, int &charIndex)
{
    if (charIndex >= data.length() )
    {
      return ""; //end of data
    }
    
    String field = "";
    bool hitDelimiter = false; // flag for delimiter detection 
    while (hitDelimiter == false) // loop characters until next delimiter
    {
        if (charIndex >= data.length() )
        {
          break; //end of data
        }

        if (String(data[charIndex]) == "\n") // if character is a line break
        {
          break; // end of data
        }
        
       if(String(data[charIndex]) == kDelimiter) // if we hit a delimiter
        {
          hitDelimiter = true;  // flag the delimiter hit
          charIndex++; // set iterator after delimiter so we skip next comma
          break;
        }
        else
        {        
          field += data[charIndex]; // add character to field string
          charIndex++; // increment to next character in data
        }
    }
    return field;
}
