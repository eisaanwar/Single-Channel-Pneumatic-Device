#define MAX_INTS 1000            //1000          // Maximum number of from input file
#define MAX_STRING_LENGTH 17000  //17000 // Maximum length of the input string

#include "stm32h7xx_hal.h" // hardware abstration layer to rest chip

//Pnewmatic Valves
const int valve1 = 10;  // Flextion pin (PWM)
const int valve2 = 11;  // Extension pin (PWM)

const int valve1Log = A1;  // postion of flex valve (analogue in)
const int valve2Log = A2;  // postion of exten valve (analogue in)

//Fibre Optic Sensor
const int fibreOptic = A3;  // fibre optic input pin
int fibOptValue = 0;

//Status LEDs (PWM pins for brightness)
const int whiteLed = 5;
const int greenLed = 6;

//MRI Input
const int mriPin = 2;         // MRI input pin
volatile byte mriSignal = 0;  // MRI singal status

//BNC Outputs
const int pulseTap = 3;      // Pulse for Tap Activation
const int pulseContact = 4;  // Pulse for Fibre Optic Contact

//Data Received From Serial
String inputString;  // Full string from control panel
int inputLength;
String inputIdent;                       // Identifier: The first 5 characters of the singal (Strt:, Wrst:, Hand:)
String inputSequence = "";               // The sequence of commands?
char inputCharArray[MAX_STRING_LENGTH];  // Char array for the data to be processed

//Sequence
String mode = "";
int sequenceLength = 0;

//pointers to the arrays (not declaring here to save memory)
int* sequenceWrist;  //[MAX_INTS];

int* sequenceHandTime;       //[MAX_INTS];
int* sequenceHandIntensity;  //[MAX_INTS];
bool autoTapTime = false;
int* sequenceHandContactTime;  //[MAX_INTS];

//Hand Tapping Consts
int tapTimout = 3000;
int fibreThreshold = 500;


void setup() {
  Serial.begin(115200);

  pinMode(valve1, OUTPUT);
  pinMode(valve2, OUTPUT);

  pinMode(whiteLed, OUTPUT);
  pinMode(greenLed, OUTPUT);

  pinMode(mriPin, INPUT);  // for MRI input

  //digitalWrite(whiteLed, HIGH);

  pinMode(pulseTap, OUTPUT);
  pinMode(pulseContact, OUTPUT);
  digitalWrite(pulseTap, LOW);
  digitalWrite(pulseContact, LOW);

  attachInterrupt(digitalPinToInterrupt(2), pulseReceived, RISING);  // interrupt to cpature MRI TTL Pulse
}

void pulseReceived() {
  mriSignal = 1;  // MRI pulse received
}

void loop() {
  inputString = "";  // reset the serial input
  inputIdent = "";

  while (Serial.available()) {  // If data is available to read,
    inputString = Serial.readStringUntil('\n');
    inputLength = inputString.length();
    inputIdent = inputString.substring(0, 5);

    Serial.print("id:");  // diagostics // but nothing when click start
    Serial.println(inputIdent);
  

    if (inputIdent.equals("Test:")) {  // Perform a Test Movement of the device
      runTestSequence();
    }

    if (inputIdent.equals("Wrst:")) {                         // Upload Button Pushed and Wrist Actuation Selected in Control Panel
      inputSequence = inputString.substring(5, inputLength);  // read the data of the input sequence in not
      mode = "WRIST";
      //Serial.print("W");

      if (sequenceWrist != nullptr) {  // if there is already something there then delete it
        delete[] sequenceWrist;        // delete old values, to save space (I dont think c automatically deletes)
      }
      sequenceWrist = new int[MAX_INTS]{ 0 };  // initialse to 0 otherise can hold randon values?

      inputSequence.toCharArray(inputCharArray, MAX_STRING_LENGTH);  // convert the input sequence to char array
      //Serial.println(inputCharArray);
      stringToIntArrayWrist(inputCharArray, sequenceWrist);  // convert the char array to int array,

      for (int i = 0; i < sequenceLength; i++) {  // for checking put in the start if going to use
        Serial.print(sequenceWrist[i]);
      }
      Serial.println("");

    } else if (inputIdent.equals("Hand:")) {                  // Upload Button Pushed and Wrist Actuation Selected in Control Panel
      inputSequence = inputString.substring(5, inputLength);  // read the data of the input sequence in not
      mode = "HAND";
      Serial.print("H");

      if (sequenceHandTime != nullptr) {  // if there is already something there then delete it
        delete[] sequenceHandTime;        // delete old values, to save space (I dont think c automatically deletes)
      }
      sequenceHandTime = new int[MAX_INTS]{ 0 };
      if (sequenceHandIntensity != nullptr) {  // if there is already something there then delete it
        delete[] sequenceHandIntensity;        // delete old values, to save space (I dont think c automatically deletes)
      }
      sequenceHandIntensity = new int[MAX_INTS]{ 0 };
      if (sequenceHandContactTime != nullptr) {  // if there is already something there then delete it
        delete[] sequenceHandContactTime;        // delete old values, to save space (I dont think c automatically deletes)
      }
      sequenceHandContactTime = new int[MAX_INTS]{ 0 };
      inputSequence.toCharArray(inputCharArray, MAX_STRING_LENGTH);  // convert the input sequence to char array
      stringToIntArrayHand(inputCharArray, sequenceHandTime, sequenceHandIntensity, sequenceHandContactTime);  // convert the char array to int array,

      for (int i = 0; i < sequenceLength; i++) {  // for checking put in the start if going to use
        Serial.print("T");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(sequenceHandTime[i]);
        Serial.print(" I");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(sequenceHandIntensity[i]);
        Serial.print(" D");
        Serial.print(i);
        Serial.print(": ");
        Serial.print(sequenceHandContactTime[i]);
      }
      Serial.println(".");
    }
  }


  if (inputIdent.equals("Strt:") && (inputLength == 6)) {  // if the start button is pressed with ardument 0 or 1
    if (inputString.substring(5,6).equals("0")) {
      Serial.println("Starting immediately");
      mriSignal = 1; // simualte already received signal

    } else if (inputString.substring(5,6).equals("1")) {
      Serial.println("Wait for MRI");
      mriSignal = 0; // reset the MRI singal to wait for scanner pulse
    } else {
      Serial.println("Incorrect Start Signal");
      
    }

    if (mode.equals("WRIST")) {
      //Serial.print("Start Wrist: ");
      runSequenceWrist(sequenceWrist, sequenceLength);

    } else if (mode.equals("HAND")) {
      //Serial.print("Start Hand: ");
      runSequenceHand(sequenceHandTime, sequenceHandIntensity, sequenceHandContactTime, sequenceLength);
    }
  }
}

void runTestSequence() {
  //pinMode(valve1, OUTPUT); // in case it was used for intensity in other one 
  int startMillis = millis();
  int previousMillis;
  dataLog(millis() - startMillis);
  previousMillis = millis();     // reset the previous
  digitalWrite(pulseTap, HIGH);  // send pulse to indicate actuation
  analogWrite(valve1, 255);
  digitalWrite(valve2, LOW);
  while (millis() - previousMillis < 2000) {
    if(dataLog(millis() - startMillis) == false) {  // data log - startMillis
        return; 
    }
  }
  previousMillis = millis();  // reset the previous
  //Serial.println("release");
  digitalWrite(pulseTap, LOW);  // send pulse to indicate release (or wrist)
  analogWrite(valve1, 0);
  digitalWrite(valve2, HIGH);
  while (millis() - previousMillis < 2000) {
    if(dataLog(millis() - startMillis) == false) {  // data log - startMillis
        return; 
    }
  }
  digitalWrite(valve2, LOW); // then both are off
  Serial.println("fin"); // when finsihed
}

void stringToIntArrayWrist(char* inputCharArray, int* sequenceWrist) {  // send the pointers
  char* token = strtok(inputCharArray, ":");
  int index = 0;
  while (token != NULL) {
    sequenceWrist[index++] = atoi(token);  // Convert the token to integer and store it in the array
    token = strtok(NULL, ":");
    //Serial.println(index);            // Get the next token
  }
  sequenceLength = index;  // number of values? another fucntion for wrist? or boolean?
}



void stringToIntArrayHand(char* inputCharArray, int* sequenceHandTime, int* sequenceHandIntensity, int* sequenceHandContactTime) {  // send the location
  char* token = strtok(inputCharArray, ":");
  int index = 0;
  while (token != NULL) {
    String SequenceLine(token);                                      // Convert the token to integer and store it in the array
    int commaLocation = SequenceLine.indexOf(',');                   // find where the comma is (to get the intensity after the time)
    int secondComma = SequenceLine.indexOf(',', commaLocation + 1);  // find the next comma after the first one (for the tap time)
    //Serial.println(commaLocation);
    //Serial.println(secondComma);
    sequenceHandTime[index] = (SequenceLine.substring(0, commaLocation)).toInt();                               // time value (stop before comma)
    sequenceHandIntensity[index] = (SequenceLine.substring(commaLocation + 1, secondComma)).toInt();            // intensity value (after comma)
    sequenceHandContactTime[index] = (SequenceLine.substring(secondComma + 1, SequenceLine.length())).toInt();  // tap time
    index = index + 1;                                                                                          // increment the index
    token = strtok(NULL, ":");                                                                                  // Get the next token
  }
  sequenceLength = index;  // number of values? another fucntion for wrist? or boolean?
}


void runSequenceHand(int* sequenceHandTime, int* sequenceHandIntensity, int* sequenceHandContactTime, int sequenceLength) {
  unsigned long startMillis;  // = millis();  // start time
  int tapTime;// = 150;          // in ms
  int tapDelay = 0;
  int startTapDelay;

  int sequenceTime;
  int intensity;

  while (mriSignal == 0) {  // wait until the MRI input is high
    digitalWrite(whiteLed, LOW);
  }
  mriSignal = 0;  // an MRI singal is received. Reset the trigger. The sequence will commence to its end
  digitalWrite(whiteLed, HIGH);
  startMillis = millis();  // start time

  for (int i = 0; i < sequenceLength; i++) {
    sequenceTime = sequenceHandTime[i];  // read the next time to tap
    tapTime = sequenceHandContactTime[i]; // if this is negativ then we have an autotime (coint using fibre
    intensity = map(sequenceHandIntensity[i], 0, 100, 0, 255);

    //Serial.println(intensity);
    while (millis() - startMillis < sequenceTime) {  //wait for hand time. then actuate wirth intensity, when move on
      if(dataLog(millis() - startMillis) == -1) {  // data log - startMillis
          return; 
      }
    }

    digitalWrite(pulseTap, HIGH);    // send pulse to indicate tap start
    analogWrite(valve1, intensity);  // tap with the desired intensity
    digitalWrite(valve2, LOW);
    Serial.println(tapTime);
    if(tapTime < 0) { // if negative then use autotiming
      tapTime = -tapTime; // invert for the delay
      startTapDelay = millis() - startMillis;
      int fibreVal = dataLog(millis() - startMillis);
      
      while (fibreVal > fibreThreshold) { // wait for fibre to connect
        Serial.print("waiting");
        fibreVal = dataLog(millis() - startMillis);
        if (fibreVal == -1) {
          return;
        }
        if( (millis() - startMillis) > sequenceTime + 3000) { // if it takes more than a second then stop waiting
          Serial.print("timeout");
          break; 
        }   
      }
      int tapDelay = (millis() - startMillis) - startTapDelay; // the time it took to initiate the tap (cannot be more than 1s)
    }

    while (millis() - startMillis < sequenceTime + tapTime + tapDelay) {  // wait the tapTime // what about auto tap time
      if(dataLog(millis() - startMillis) == -1) {  // data log - startMillis
          return; 
      }
    }
    tapDelay = 0; // reset this for the next one
    digitalWrite(pulseTap, LOW);  // stop the start tap pulse
    analogWrite(valve1, LOW);     // release
    digitalWrite(valve2, HIGH);
  }

  Serial.println("fin");
  
  digitalWrite(pulseContact, LOW);  // low pulse value on fibre when finished
  digitalWrite(valve2, HIGH);
  delay(500);
  analogWrite(valve1, LOW);  // close both valves at the end
  digitalWrite(valve2, LOW);
  digitalWrite(whiteLed, LOW);  // to show the system has finished
  //pinMode(valve1, OUTPUT); // so it can again be controlled by digital
  
}


void runSequenceWrist(int* sequenceWrist, int sequenceLength) {
  //pinMode(valve1, OUTPUT); // in case it was used for intensity in other one 
  unsigned long startMillis;      // = millis();  // timer
  unsigned long previousMillis;   // = millis();
  bool firstSigRecieved = false;  // to start the timer on the first pulse

  int flextionTime = 500;   // Value 1 time in ms (bend wrist (palm down moves down))
  int extensionTime = 500;  // Valve 2 (extend wrist palm down move wrist up))
  int mriTime = 500;        // time the MRI signal is high, need to wait for next signal

  for (int i = 0; i < sequenceLength; i++) {
    while (mriSignal == 0) {  // wait until the MRI input is high
      digitalWrite(whiteLed, LOW);
      if (firstSigRecieved == true) {     // only data log when the first signal has been received to start the sequence
        if(dataLog(millis() - startMillis) == -1) {  // data log - startMillis
          return; 
        }
      }
    }

    mriSignal = 0;  // an MRI singal is received
    digitalWrite(whiteLed, HIGH);
    if (firstSigRecieved == false) {  // this tis the first signal from the MRI
      firstSigRecieved = true;        // to show th efirst signal is receiced
      startMillis = millis();         // start the timer
      previousMillis = millis();      // the interval timer
    }

    if (sequenceWrist[i] == 1) {  // if the sequence actuates on this signal
      //Serial.println("push");
      previousMillis = millis();     // reset the previous
      digitalWrite(pulseTap, HIGH);  // send pulse to indicate actuation
      analogWrite(valve1, 255);
      digitalWrite(valve2, LOW);
      while (millis() - previousMillis < flextionTime) {
        if(dataLog(millis() - startMillis) == -1) {  // data log - startMillis
          return; 
        }
      }
      previousMillis = millis();  // reset the previous
      //Serial.println("release");
      digitalWrite(pulseTap, LOW);  // send pulse to indicate release (or wrist)
      analogWrite(valve1, 0);
      digitalWrite(valve2, HIGH);
      while (millis() - previousMillis < extensionTime) {
        dataLog(millis() - startMillis);  // data log
      }
    } else {
      //Serial.println("skip");
      previousMillis = millis();                     // reset the previous
      while (millis() - previousMillis < mriTime) {  // time for Mri to go low again
        if(dataLog(millis() - startMillis) == -1) {  // data log - startMillis
          return; 
        }
      }
    }
  }

  Serial.println("fin");
  digitalWrite(pulseContact, LOW);  // low pulse value on fibre when finished
  analogWrite(valve1, 0);        // close both valves at the end
  digitalWrite(valve2, LOW);
  digitalWrite(whiteLed, LOW);  // to show the system has finished
}


int dataLog(int time) {  // time and hand postition, or tap status
  while (Serial.available()) {  // If data is available to read,
    inputString = Serial.readStringUntil('\n'); //reset ship if stop is received
    if(inputString.equals("STOP")) {
      digitalWrite(pulseContact, LOW);  // low pulse value on fibre when finished
      analogWrite(valve1, 0);        // close both valves at the end
      digitalWrite(valve2, LOW);
      digitalWrite(whiteLed, LOW);  // to show the system has finished
      return -1; //NVIC_SystemReset(); 
    }
  }
  Serial.print("T,");
  Serial.print(time);
  Serial.print(", F,");
  fibOptValue = analogRead(fibreOptic);
  Serial.println(fibOptValue);
  if (fibOptValue <= fibreThreshold) {
    digitalWrite(pulseContact, HIGH);  // send pulse to indicate contact
    //Serial.println("CONTACT");
  } else {
    digitalWrite(pulseContact, LOW);  // send pulse to indicate no contact
  }
  delay(1); // microseconds??? or get program to wait
  return fibOptValue;
}
