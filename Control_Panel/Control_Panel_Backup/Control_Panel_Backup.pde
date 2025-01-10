import processing.serial.*;
PFont f;                           // STEP 1 Declare PFont variable

Serial myPort;  // Create object from Serial class
int val;        // Data received from the serial port

String inputSequence;    // to store the inoutFile

String[] inputFile;

void setup() {
  selectInput("Select a file to process:", "fileSelected"); // global variable rn
  
  size(500, 500);
  // I know that the first port in the serial list on my mac
  // is always my  FTDI adaptor, so I open Serial.list()[0].
  // On Windows machines, this generally opens COM1.
  // Open whatever port is the one you're using.
  String portName = Serial.list()[1];
  myPort = new Serial(this, portName, 9600); // is there a way to use a menu to choose the port?
  
  f = createFont("Arial",16,true); // STEP 2 Create Font

}

void draw() {
  background(255);
  textFont(f,32);                  // STEP 3 Specify font to be used
  textAlign(CENTER);
  fill(0);                         // STEP 4 Specify font color
  text("Pneumatic Control Panel",250,35);   // STEP 5 Display Text
  
  //start button
  if (mouseOverStart() == true) { // If mouse is over square,
    fill(0, 220, 0);              // change color and
    //myPort.write('H');            // send an H to indicate mouse is over square
  } else {                        // If mouse is not over square,
    fill(0, 190, 0);              // change color and
    //myPort.write('L');             // send an L otherwise
  }
  rect(50, 50, 100, 100);         // Draw start button
  textFont(f,20);                 // STEP 3 Specify font to be used
  textAlign(CENTER);
  fill(255);                      // STEP 4 Specify font color
  text("Start",100,100);          // STEP 5 Display Text
  
  //send button
  if (mouseOverSend() == true) { // If mouse is over square,
    fill(200, 200, 200);              // change color and
    //myPort.write('H');            // send an H to indicate mouse is over square
  } else {                        // If mouse is not over square,
    fill(150, 150, 150);              // change color and
    //myPort.write('L');             // send an L otherwise
  }
  rect(200, 50, 100, 100);        // Draw stop button
  textFont(f,20);                 // STEP 3 Specify font to be used
  textAlign(CENTER);
  fill(255);                         // STEP 4 Specify font color
  text("Send",250,100);   // STEP 5 Display Text
  
}


void mousePressed() {
  if (mouseOverStart()) { // if pressed start
    fill(0,140,0);                // change color and
    myPort.write("strt:");            // send an H to indicate mouse is over square
    rect(50, 50, 100, 100);         // Draw start button
    textFont(f,20);                 // STEP 3 Specify font to be used
    textAlign(CENTER);
    fill(255);                      // STEP 4 Specify font color
    text("Start",100,100);          // STEP 5 Display Text
  }
  
  if (mouseOverSend()) { // if pressend send
    fill(0,140,0);                // change color and
    myPort.write();            // send an H to indicate mouse is over square
    rect(50, 50, 100, 100);         // Draw start button
    textFont(f,20);                 // STEP 3 Specify font to be used
    textAlign(CENTER);
    fill(255);                      // STEP 4 Specify font color
    text("Start",100,100);          // STEP 5 Display Text
  }
}


String fileSelected(File selection) {
  if (selection == null) {
    println("Window was closed or the user hit cancel.");
  } else {
    println("Input File Location: " + selection.getAbsolutePath());
  }
  //inputFile = loadTable(selection.getAbsolutePath(), "header");
  //reader = createReader("positions.txt"); 
  //println(inputFile.getInt(0,0));
  //String[] inputFile;
  inputFile = loadStrings(selection.getAbsolutePath());
  println("there are " + inputFile.length + " lines");
  //for (int i = 0 ; i < inputFile.length; i++) {
  //  println(inputFile[i]);
  //}
  inputSequence = "sequ:" + join(inputFile, "");
  println(inputSequence);

  return inputSequence; // dont need to return rn as its a global variable
}


boolean mouseOverStart() { // Test if mouse is over square
  return ((mouseX >= 50) && (mouseX <= 150) && (mouseY >= 50) && (mouseY <= 150));
}

boolean mouseOverSend() { // Test if mouse is over square
  return ((mouseX >= 200) && (mouseX <= 300) && (mouseY >= 50) && (mouseY <= 150));
}
