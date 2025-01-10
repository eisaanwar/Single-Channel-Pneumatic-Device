import processing.serial.*;
PFont f;                           // STEP 1 Declare PFont variable

Serial myPort;  // Create object from Serial class
int val;        // Data received from the serial port

//date and time for log file
String second = String.valueOf(second());  // Values from 0 - 59
String minute = String.valueOf(minute());  // Values from 0 - 59
String hour = String.valueOf(hour());    // Values from 0 - 23
String day = String.valueOf(day());    // Values from 1 - 31
String month = String.valueOf(month());  // Values from 1 - 12
String year = String.valueOf(year());   // 2003, 2004, 2005, etc.

PrintWriter outputFile; // Writer for output file;


String mode = "";
String uploadStatus = "";

//Interface and Buttons
int winWidth = 500;
int winLength = 600;

int[] wristButton = {75, 160, 150, 50};
int[] tapButton = {275, 160, 150, 50};

int[] inputButton = {50, 250, 150, 50};
int[] outputButton = {50, 325, 150, 50};

int[] testButton = {175, 425, 150, 50};
int[] uploadButton = {175, 500, 150, 50};
int[] startButton = {175, 575, 150, 50}; // {xPos, yPos, width, hight}

//Serial
String serialReceive = "";

String inputLocation = "";
String inputSequence = "";    // to store the inputFile
String uploadSequence = ""; // to send to the arduino
String outputLocation = ""; // folder to store the output data received from arduino


void setup() {

  size(500, 650); // window size

  //Serial
  // I know that the first port in the serial list on my mac is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  String portName = Serial.list()[1];
  myPort = new Serial(this, portName, 115200); // is there a way to use a menu to choose the port?
  println(Serial.list());

  //Font
  f = createFont("Arial", 16, true); // STEP 2 Create Font
}

void draw() {
  background(255);
  textFont(f, 32);                 // STEP 3 Specify font to be used
  textAlign(CENTER);
  fill(0);                         // STEP 4 Specify font color
  text("Pneumatic Control Panel", 250, 40);   // STEP 5 Display Text
  line(10, 55, winWidth-10, 55);
  textFont(f, 20);

  // read any incomming serial
  while (myPort.available() >0) {  // If data is available to read,
    serialReceive = myPort.readStringUntil('\n'); // trim to remove new line (but shouldnt be included?
    println(serialReceive);
    if(serialReceive == null) { // why would it be null though (srial skipped?)
      serialReceive = "";
      break; // leave the loop
    }
    serialReceive = serialReceive.trim();

    if (serialReceive.substring(0,1).equals("D")) { // if its data to log
      println("Received Data Signal");
      dataLog(serialReceive.substring(1, serialReceive.length())); //log
    }
    if (serialReceive.equals("fin")){ // why do I need to trim here??????

      println("Received Finish Signal");
      outputFile.flush(); // Write the remaining data
      outputFile.close(); // Close the file
    }
  }

  textAlign(LEFT);
  fill(0);
  text("Serial Port", 50, 97);
  text(serialReceive, 150, 97); // received serial data

  line(10, 125, winWidth-10, 125);
  fill(0);
  text("Mode: "+mode, 50, 150);

  drawButton(wristButton, "Wrist Actuation", new int[]{150, 150, 150});
  drawButton(tapButton, "Hand Tapping", new int[]{150, 150, 150});

  line(10, 225, winWidth-10, 225);

  fill(0);
  drawButton(inputButton, "Input Location", new int[]{150, 50, 150}); // input
  fill(0);
  textAlign(LEFT);
  textFont(f, 10);
  text(inputLocation, 10, 310);

  drawButton(outputButton, "Output Location", new int[]{150, 50, 150}); // output
  fill(0);
  textAlign(LEFT);
  textFont(f, 10);
  text(outputLocation, 10, 385);

  line(10, 400, winWidth-10, 400);
  
  drawButton(testButton, "Test", new int[]{190, 190, 50});

  drawButton(uploadButton, "Upload", new int[]{0, 50, 190});
  fill(0);
  textAlign(LEFT);
  textFont(f, 15);
  text(uploadStatus, 340, 455);

  drawButton(startButton, "Start", new int[]{0, 150, 0});
}

void dataLog(String data) {
  outputFile.println(data);// + "\t" + mouseY);
}

void mouseClicked() {
  if (mouseOverButton(wristButton)) { // if pressend send
    mode = "Wrist Actuation";
    println("wrist actuate mode:");
  } else if (mouseOverButton(tapButton)) { // if pressend send
    mode = "Hand Tapping";
    println("hand tap mode");
  } else if (mouseOverButton(testButton)) { // if pressend send
    println("test");
    myPort.write("Test:");
    // save to file (test)
  } else if (mouseOverButton(outputButton)) { // output folder
    selectFolder("Select a folder to process:", "folderSelected"); // global variable rn
    

    // create file to save data
  } else if (mouseOverButton(inputButton)) { // input file
    selectInput("Select a file to process:", "fileSelected"); // global variable rn
  } else if (mouseOverButton(uploadButton)) { // if pressend upload
    // upload message, need to select mode and files
    if (modeCheck() == false) { // check if user selected a mode
      uploadStatus = "Please Select A Mode";
      return;
    } else {
      uploadStatus = "";
    }
    if (fileLocationCheck() == false) {
      return;// a check failed
    } else {
      uploadStatus = "";
    }
    if (mode.equals("Wrist Actuation")) {
      uploadSequence = "Wrst:" +  inputSequence; // for wrist actuation
      // also need the time dealy here to upload
      println(uploadSequence);
      myPort.write(uploadSequence);
    } else if (mode.equals("Hand Tapping")) {
      uploadSequence = "Hand:" +  inputSequence; // for hand tapping
      println(uploadSequence);
      myPort.write(uploadSequence);
    }
  } else if (mouseOverButton(startButton)) { // if pressed start
    myPort.write("Strt:");            // send an H to indicate mouse is over square
    println("Strt:");
  }
  //maybe a draw pressed button method in a class
}

boolean modeCheck() {
  // check if mode/ input folder/ output folder/ time delay if (wrist, or in file?)/ an if connected
  // do any check on the file or input sequence? (after these have passed
  if (mode.equals("")) {
    println("No mode selected");
    //inputLocation = "Please Select A Mode";
    return false;
  } else {
    //inputLocation = ""; // remove the string
    return true;
  }
}

boolean fileLocationCheck() {
  if (inputLocation.equals("")) {
    println("No input folder selected");
    uploadStatus = "Please Select An Input Folder";
    return false;
  }
  if (outputLocation.equals("")) {
    println("No output folder selected");
    uploadStatus = "Please Select An Output Folder";
    return false;
  }
  return true; // all check passed
}


String fileSelected(File selection) { // input file
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
  } else {
    inputLocation = selection.getAbsolutePath();
    println("Input File Location: " + inputLocation);
  }
  String[] inputFile = loadStrings(selection.getAbsolutePath());
  println("there are " + inputFile.length + " lines");
  inputSequence = join(inputFile, "-"); //"seq:"
  println(inputSequence);
  return inputSequence; // dont need to return rn as its a global variable
}

void folderSelected(File selection) { // output folder
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
  } else {
    outputLocation = selection.getAbsolutePath();
    println("User selected " + outputLocation);
  }
  //Output File
  String fileName = hour+"-"+minute+"-"+second+"_"+day+"-"+month+"-"+year;
  outputFile = createWriter(outputLocation + "/" + fileName);
}


void drawButton (int[] buttonSizePos, String text, int[] colour) {
  if (mouseOverButton(buttonSizePos) == true) { // If mouse is over square, (also see if click?)
    if (mousePressed == true) { // if mouse clicked on this
      fill(colour[0]+70, colour[1]+70, colour[2]+70);
    } else { // just hovering over
      fill(colour[0]+30, colour[1]+30, colour[2]+30);
    }
  } else {                        // If mouse is not over square,
    fill(colour[0], colour[1], colour[2]);// default colour
  }
  rect(buttonSizePos[0], buttonSizePos[1], buttonSizePos[2], buttonSizePos[3]);// Draw start button
  fill(255);
  textAlign(CENTER);
  textFont(f, 20);
  text(text, buttonSizePos[0]+75, buttonSizePos[1]+32);   // STEP 5 Display Text
}

boolean mouseOverButton(int[] buttonSizePos) { // Test if mouse is over a button
  return ((mouseX >= buttonSizePos[0]) && (mouseX <= buttonSizePos[0]+buttonSizePos[2]) && (mouseY >= buttonSizePos[1]) && (mouseY <= buttonSizePos[1]+buttonSizePos[3]));
}
