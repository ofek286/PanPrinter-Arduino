/*
 * The main program for the PanPrinter system.
 * 
 * Version: 01/03/2016 (beta)
 * 
 * /------------\
 * | A          |
 * | ♠          |
 * |     /\     |
 * |    /  \    |
 * |   /____\   |
 * |  /      \  |
 * | /        \ |
 * |            |
 * |           A|
 * |           ♠|
 * \------------/
 *
 */
 
#include <Servo.h>
#include <SPI.h>
#include <SD.h>
 
Servo xRightAxisServo;
Servo xLeftAxisServo;
Servo yAxisServo;

int xRightAxisServoPin = 10;
int xLeftAxisServoPin = 11;
int yAxisServoPin = 12;

int xRSpectrum = 50;
int xLSpectrum = 50;
int ySpectrum = 40;

int xRFixer = 70;
int xLFixer = 80;
int yFixer = 0;

boolean OSGrid;
int maxLevel;
int currLevel = 0;
boolean durLevel = false;

double degreesPerXR;
double degreesPerXL;
double degreesPerY;

int currX;
int currY;

int maxX;
int maxY;

boolean adminModeStatus = false;

boolean restart = false;

File myFile;
	
void setup() {
  Serial.begin(9600);
  xRightAxisServo.attach(xRightAxisServoPin);
  xLeftAxisServo.attach(xLeftAxisServoPin);
  yAxisServo.attach(yAxisServoPin);
}

void loop() {
  if (restart)
  {
    restarter();
  }
  if (Serial.available() > 0)
  {
    cmdReciever(Serial.readString());
  }
  delay(500);
}

/*
 * The OSGrid Parser
 */
void cmdReciever(String dirtyCmd) {
  String args[2];
  String cmd;
  
  //Splitting the full cmd to cmd and args
  if (dirtyCmd.indexOf(" ") != -1)
  {
    if (dirtyCmd.indexOf(",") != -1)
    {
      cmd = dirtyCmd.substring(0, dirtyCmd.indexOf(' '));
      args[0] = dirtyCmd.substring(dirtyCmd.indexOf(' ') + 1, dirtyCmd.indexOf(','));
      args[1] = dirtyCmd.substring(dirtyCmd.indexOf(',') + 1);
    }
    else
    {
      cmd = dirtyCmd.substring(0, dirtyCmd.indexOf(' '));
      args[0] = dirtyCmd.substring(dirtyCmd.indexOf(' ') + 1);
    }
  }
  else
  {
    cmd = dirtyCmd;
  }
  
  //Checking what is the command
  if (cmd.equals("helloPanSys")) // Connection establishment
  {
    Serial.println("Hello User");
    reset();
  }
  else if (cmd.equals("checkCon")) // Connection check
    conChecker(args[0]);
  else if (cmd.equals("defMethod")) // Print language chooser
    methodDef(args[0]);
  else if (cmd.equals("gridDef")) // Grid size
    gridSizer(args[0]);
  else if (cmd.equals("levelsDef")) // Hit levels
    maxLeveler(args[0]);
  else if (cmd.equals("startPos")) // Moving to start position
    resetLevelPos(args[0], args[1]);
  else if (cmd.equals("doLevel")) // Opening the mix chamber
    levelStarter(args[0]);
  else if (cmd.equals("movTo")) // Moving
    moveLevel(args[0], args[1]);
  else if (cmd.equals("endLevel")) // Closing the mix chamber
    levelEnder();
  else if (cmd.equals("endPrint")) // Finishing up
    goodBye(0);
  else if (cmd.equals("helloOSsys")) // Admin mode
    adminMode();
  else if (cmd.equals("printFile")) // Running a print file
    printFile(args[0]);
}

/*
 * Calibrating the printer
 */
void reset() {
  move(0,0);
	
  currX = 0;
  currY = 0;

  maxX = 0;
  maxY = 0;

  OSGrid = false;
  maxLevel = 0;
  currLevel = 0;
  durLevel = false;
}

/*
 * Checking the connection
 */
void conChecker(String confS) {
  int conf = confS.toInt();
  Serial.println("OK " + String(conf) + " received");
}

/*
 * Choosing the print language
 */
void methodDef(String method) {
  if (method.equals("OSGrid"))
    OSGrid = true;
  Serial.println("OK " + method + " will be used");
}

/*
 * Setting the grid size, and aligning the printer accordingly
 */
void gridSizer(String gridSize) {
  if (!OSGrid)
  {
    Serial.println("OSGrid is not defined");
  }
  else
  {
    if (gridSize.equals("16C"))
    {
      degreesPerXR = xRSpectrum/15;
      degreesPerXL = xLSpectrum/15;
      degreesPerY = ySpectrum/15;
      maxX = 15;
      maxY = 15;			
      Serial.println("OK Grid Size is " + gridSize);
    }
    else if (gridSize.equals("15C"))
    {
      degreesPerXR = xRSpectrum/14;
      degreesPerXL = xLSpectrum/14;
      degreesPerY = ySpectrum/14;
      maxX = 14;
      maxY = 14;
      Serial.println("OK Grid Size is " + gridSize);
    }
    else if (gridSize.equals("18C"))
    {
      degreesPerXR = xRSpectrum/17;
      degreesPerXL = xLSpectrum/17;
      degreesPerY = ySpectrum/17;	
      maxX = 17;
      maxY = 17;
      Serial.println("OK Grid Size is " + gridSize);
    }
    else if (gridSize.equals("19C"))
    {
      degreesPerXR = xRSpectrum/18;
      degreesPerXL = xLSpectrum/18;
      degreesPerY = ySpectrum/18;
      maxX = 18;
      maxY = 18;
      Serial.println("OK Grid Size is " + gridSize);
    }
  }
}

/*
 * Limiting the hit levels
 */
void maxLeveler(String levelS) {
  if (!OSGrid)
  {
    Serial.println("OSGrid is not defined");
  }
  else
  {
    maxLevel = levelS.toInt();
    Serial.println("OK " + String(maxLevel) + " is the levels amount");
  }
}

/*
 * Moving to the starting location of the current hit level
 */
void resetLevelPos(String xLocS, String yLocS) {
  if (adminModeStatus)
  {
    Serial.println("Admin mode activated, can't print");
    return;
  }
  int xLoc = xLocS.toInt();
  int yLoc = yLocS.toInt();
  if (move(xLoc, yLoc))
  {
	currLevel++;
	Serial.println("OK beginning level on (" + String(xLoc) + "," + String(yLoc) + ")");
  }
  else
  {
	Serial.println("Error - beyond grid");
  }
}

/*
 * Starting the new hit level, and checking if it's OK
 */
void levelStarter(String currLevelS) {
  boolean bool1 = currLevel != currLevelS.toInt();
  boolean bool2 = currLevel > maxLevel;
  boolean bool3 = durLevel == true;
  boolean bool4 = adminModeStatus;
  if (bool1)
  {
    Serial.println("Current level is different then expected. Should be " + String(currLevel));
  }
  else if (bool2)
  {
    Serial.println("Current level tops max level, the system will shut down.");
    goodBye(2);
  }
  else if (bool3)
  {
    Serial.println("Last level was not closed");
  }
  else if (bool4)
  {
    Serial.println("Admin mode activated, can't print");
  }
  else
  {
    durLevel = true;
    Serial.println("OK beginning level " + String(currLevel));
  }
}

/*
 * Moving the printer, during the hit level
 */
void moveLevel(String xLocS, String yLocS) {
  if (adminModeStatus)
  {
    Serial.println("Admin mode activated, can't print");
    return;
  }
  int xLoc = xLocS.toInt();
  int yLoc = yLocS.toInt();
  
  if (move(xLoc, yLoc))
    Serial.println("OK");
  else
    Serial.println("Error - beyond grid");
}

/*
 * Wrapping things up, and waiting for a new print
 */
void goodBye(int errorCode) {
  switch (errorCode)
  {
    // 0 = Successful run
    case 0: Serial.println("Goodbye!"); restart = true; break; 
    
    // 2 = User tried to do a level while the max was already made
    case 2: Serial.println("Exit code: " + String(errorCode)); restart = true; break; 
    
    // 19 = Admin mode logout
    case 19: Serial.println("Logging off Admin Mode"); adminModeStatus = false; restart = true; break;
  }
}

/*
 * Finishing the current hit level
 */
void levelEnder() {
  Serial.println("OK ending level " + String(currLevel));
  durLevel = false;
}

/*
 * Admin mode activator - for debugging
 */
void adminMode() {
  adminModeStatus = true;
  Serial.println("Hello OS");
  while (!restart)
  {
    if (Serial.available() > 0)
      adminCmdReciever(Serial.readString());
  }
}

/*
 * The real code that moves the printer
 */
boolean move(int xLoc, int yLoc) {
  if (xLoc <= maxX && yLoc <= maxY)
  {
    xRightAxisServo.write(xRFixer + (xLoc * degreesPerXR));
    xLeftAxisServo.write(xLFixer + (xLoc * degreesPerXL));
    yAxisServo.write(yFixer + (yLoc * degreesPerY));
    
    currX = xLoc;
    currY = yLoc;
    return true;
  }
  else
  {
    return false;
  }
}

/*
 * Cleaner to be used after a print
 */
void restarter() {
  maxLevel = -1;
  currLevel = 0;
  durLevel = false;
  move(0,0);
  
  degreesPerXR = 0;
  degreesPerXL = 0;
  degreesPerY = 0;

  currX = -1;
  currY = -1;

  maxX = -1;
  maxY = -1;

  adminModeStatus = false;

  restart = false;
}

/*
 * Running a print file from the SD card
 */
void printFile(String fileName) {
  myFile.close();
  myFile = SD.open(("prints/" + fileName).c_str());
  if (!myFile)
  {
    if (!SD.begin(53))
    {
      Serial.println("SD Card Reader Initialization failed!");
      return;
    }
  }
  
  if (myFile)
  {
    String currLine = "";
    while (myFile.available())
    {
      currLine = myFile.readStringUntil('\n');
      Serial.println(currLine);
      cmdReciever(currLine);
      delay(500);
    }
    
    myFile.close();
  }
  else
  {
    myFile = SD.open(("prints/" + fileName).c_str());
    if (!myFile)
    {
      if (!SD.begin(53))
      {
        Serial.println("SD Card Reader Initialization failed!");
        return;
      }
    }
    
    if (myFile)
    {
      String currLine = "";
      while (myFile.available())
      {
        currLine = myFile.readStringUntil('\n');
        Serial.println(currLine);
        if (currLine.startsWith("endPrint"))
          cmdReciever("endPrint");
        else
          cmdReciever(currLine);
        delay(500);
      }
      
      myFile.close();
    }
    else
      Serial.println("Error reading " + fileName);
  }
}

/*
 * Admin mode commands parser
 */
void adminCmdReciever(String dirtyCmd) {
  String args[2];
  String cmd;
  //Command splitter
  if (dirtyCmd.indexOf(" ") != -1)
  {
    if (dirtyCmd.indexOf(",") != -1)
    {
      cmd = dirtyCmd.substring(0, dirtyCmd.indexOf(' '));
      args[0] = dirtyCmd.substring(dirtyCmd.indexOf(' ') + 1, dirtyCmd.indexOf(','));
      args[1] = dirtyCmd.substring(dirtyCmd.indexOf(',') + 1);
    }
    else
    {
      cmd = dirtyCmd.substring(0, dirtyCmd.indexOf(' '));
      args[0] = dirtyCmd.substring(dirtyCmd.indexOf(' ') + 1);
    }
  }
  else
  {
    cmd = dirtyCmd;
  }
  
  if (cmd.equals("checkCon"))
    conChecker(args[0]);
  else if (cmd.equals("defMethod"))
    methodDef(args[0]);
  else if (cmd.equals("gridDef"))
    gridSizer(args[0]);
  else if (cmd.equals("movTo")) // Mover
    moveAdmin(args[0], args[1]);
  else if (cmd.equals("printInfo")) // Info about the printer
    adminPrintInfo();
  else if (cmd.equals("printLoc")) // Info about the printer's location in the grid
    adminPrintLoc();
  else if (cmd.equals("read")) // Info about the motors
    adminRead();
  else if (cmd.equals("signature")) // Easter egg - signature
    adminSig();
  else if (cmd.equals("x.write")) // Writing values to the motors directly
    adminWrite("x", args[0]);
  else if (cmd.equals("xRight.write")) // Writing values to the motors directly
    adminWrite("xR", args[0]);
  else if (cmd.equals("xLeft.write")) // Writing values to the motors directly
    adminWrite("xL", args[0]);
  else if (cmd.equals("y.write")) // Writing values to the motors directly
    adminWrite("y", args[0]);
  else if (cmd.equals("about")) // Easter egg - about
    about();
  else if (cmd.equals("logoff")) // Logging off the admin mode
    goodBye(19);
}

/*
 * Moving the printer, in admin mode
 */
void moveAdmin(String xLocS, String yLocS) {
  int xLoc = xLocS.toInt();
  int yLoc = yLocS.toInt();
  
  if (move(xLoc, yLoc))
    Serial.println("OK");
  else
    Serial.println("Error - beyond grid");
}

/*
 * Getting info about the printer
 */
void adminPrintInfo() {
  Serial.println("Location:");
  Serial.println("Supposed to be: (" + String(currX) + "," + String(currY) + ")");
  Serial.println("Is: (" + String((xRightAxisServo.read() - xRFixer)/degreesPerXR) + "," + String((yAxisServo.read() - yFixer)/degreesPerY) + ")");
  Serial.println("#########");
  Serial.println("Servo motor degrees:");
  Serial.println("X axis (R) readings: " + String(xRightAxisServo.read()));
  Serial.println("X axis (L) readings: " + String(xLeftAxisServo.read()));
  Serial.println("Y axis readings: " + String(yAxisServo.read()));
}

/*
 * Getting info about the printer location in the grid
 */
void adminPrintLoc() {
  Serial.println("Supposed to be: (" + String(currX) + "," + String(currY) + ")");
  Serial.println("Is: (" + String((xRightAxisServo.read() - xRFixer)/degreesPerXR) + "," + String((yAxisServo.read() - yFixer)/degreesPerY) + ")");
}

/*
 * Getting info about the printer's motors
 */
void adminRead() {
  Serial.println("Servo motor degrees:");
  Serial.println("X axis (R) readings: " + String(xRightAxisServo.read()));
  Serial.println("X axis (L) readings: " + String(xLeftAxisServo.read()));
  Serial.println("Y axis readings: " + String(yAxisServo.read()));
}

/*
 * Easter egg - signature
 */
void adminSig() {
  Serial.println("/---------------\\");
  Serial.println("| A              |");
  Serial.println("|  /---------\\  |");
  Serial.println("|   |  |  /  |   |");
  Serial.println("|   |  | /   |   |");
  Serial.println("|   |  |/    |   |");
  Serial.println("|   |  |\    |   |");
  Serial.println("|   |  | \   |   |");
  Serial.println("|   |  |  \  |   |");
  Serial.println("|  \\---------/  |");
  Serial.println("|               A|");
  Serial.println("\\---------------/");
}

/*
 * Writing values directly to the motors (admin mode only)
 */
void adminWrite(String motor, String degS) {
  float deg = degS.toFloat();
  if (motor.equals("xR"))
  {
    xRightAxisServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to X (R) motor");
  }
  if (motor.equals("xL"))
  {
    xLeftAxisServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to X (L) motor"); 
  }
  else if (motor.equals("y"))
  {
    yAxisServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to Y motor");
  }
  else if (motor.equals("x"))
  {
    xRightAxisServo.write(deg);
    xLeftAxisServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to X motors");
  }
}

/*
 * Easter egg - about
 */
void about() {
  Serial.println("This is the arduino-side\nsoftware for the PanPrinter machine");
  Serial.println("Last Update: 01/03/2016");
  Serial.println("Programmer: OS");
}
