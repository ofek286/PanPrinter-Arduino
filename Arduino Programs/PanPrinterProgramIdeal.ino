/*
 * The main program for the PanPrinter system.
 * 
 * Version: 28/02/2016 (alpha)
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
 
Servo xAxisServo;
Servo yAxisServo;
Servo plugServo;

//int xAxisServoPin = 10;
//int yAxisServoPin = 11;
//int plugServoPin = 12;

//int sdReaderPin = 4;

//String currMethod;
//String currGrid;
boolean OSGrid;
int maxLevel;
int currLevel = 0;
boolean durLevel = false;

double degreesPerX;
double degreesPerY;

//In the case of falsely located motors
//double resetedDegreesX;
//double resetedDegreesY;

int currX;
int currY;

int maxX;
int maxY;

boolean adminModeStatus = false;

boolean restart = false;

//int PLUG_OPEN_DEGREES = 90;
//int PLUG_CLOSE_DEGREES = 0;

File myFile;
	
void setup() {
  Serial.begin(9600);
  xAxisServo.attach(10);
  yAxisServo.attach(11);
  plugServo.attach(12);
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

void cmdReciever(String dirtyCmd) {
  String args[2];
  String cmd;
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
  
  if (cmd.equals("helloPanSys"))
  {
    Serial.println("Hello User");
    reset();
  }
  else if (cmd.equals("checkCon"))
    conChecker(args[0]);
  else if (cmd.equals("defMethod"))
    methodDef(args[0]);
  else if (cmd.equals("gridDef"))
    gridSizer(args[0]);
  else if (cmd.equals("levelsDef"))
    maxLeveler(args[0]);
  else if (cmd.equals("startPos"))
    resetLevelPos(args[0], args[1]);
  else if (cmd.equals("doLevel"))
    levelStarter(args[0]);
  else if (cmd.equals("movTo"))
    moveLevel(args[0], args[1]);
  else if (cmd.equals("endLevel"))
    levelEnder();
  else if (cmd.equals("endPrint"))
    goodBye(0);
  else if (cmd.equals("helloOSsys"))
    adminMode();
  else if (cmd.equals("printFile"))
    printFile(args[0]);
}

void reset() {
  //xAxisServo.write(resetedDegreesX);
  //yAxisServo.write(resetedDegressY);
	
  move(0,0);
  plugControl(false);
	
  currX = 0;
  currY = 0;

  maxX = 0;
  maxY = 0;

  OSGrid = false;
  maxLevel = 0;
  currLevel = 0;
  durLevel = false;
}

void conChecker(String confS) {
  int conf = confS.toInt();
  Serial.println("OK " + String(conf) + " received");
}

void methodDef(String method) {
  if (method.equals("OSGrid"))
    OSGrid = true;
  Serial.println("OK " + method + " will be used");
}

void gridSizer(String gridSize) {
  if (!OSGrid)
  {
    Serial.println("OSGrid is not defined");
  }
  else
  {
    //currGrid = gridSize;
    if (gridSize.equals("16C"))
    {
      degreesPerX = 180/15;
      degreesPerY = 180/15;
      maxX = 15;
      maxY = 15;			
      Serial.println("OK Grid Size is " + gridSize);
    }
    else if (gridSize.equals("15C"))
    {
      degreesPerX = 180/15;
      degreesPerY = 180/15;
      maxX = 14;
      maxY = 14;
      Serial.println("OK Grid Size is " + gridSize);
    }
    else if (gridSize.equals("18C"))
    {
      degreesPerX = 180/18;
      degreesPerY = 180/18;	
      maxX = 17;
      maxY = 17;
      Serial.println("OK Grid Size is " + gridSize);
    }
    else if (gridSize.equals("19C"))
    {
      degreesPerX = 180/18;
      degreesPerY = 180/18;
      maxX = 18;
      maxY = 18;
      Serial.println("OK Grid Size is " + gridSize);
    }
  }
}

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
    plugControl(true);
    Serial.println("OK beginning level " + String(currLevel));
  }
}

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

void levelEnder() {
  Serial.println("OK ending level " + String(currLevel));
  plugControl(false);
  durLevel = false;
}

void adminMode() {
  adminModeStatus = true;
  Serial.println("Hello OS");
  while (!restart)
  {
    if (Serial.available() > 0)
      adminCmdReciever(Serial.readString());
  }
}

boolean move(int xLoc, int yLoc) {
  if (xLoc <= maxX && yLoc <= maxY)
  {
    xAxisServo.write(xLoc * degreesPerX);
    yAxisServo.write(yLoc * degreesPerY);
    
    currX = xLoc;
    currY = yLoc;
    return true;
  }
  else
  {
    return false;
  }
}

void restarter() {
  //currMethod = "";
  //currGrid = "";
  maxLevel = -1;
  currLevel = 0;
  durLevel = false;
  move(0,0);
  plugControl(false);
  
  degreesPerX = 0;
  degreesPerY = 0;

  //In the case of falsely located motors
  //resetedDegreesX = -1;
  //resetedDegreesY = -1;

  currX = -1;
  currY = -1;

  maxX = -1;
  maxY = -1;

  adminModeStatus = false;

  restart = false;
}

void plugControl(boolean open) {
  if (open)
    plugServo.write(90);
  else
    plugServo.write(0);
}

void printFile(String fileName) {
  myFile.close();
  //fileName = "prints/" + fileName;
  //const char* tempName = fileName.c_str();
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
      /*
      char temp = myFile.read();
      if (temp == '\n')
      {
        Serial.println("EOL char is: " + String((int) temp));
        //Serial.println(currLine);
        cmdReciever(currLine);
        currLine = "";
      }
      else
        currLine += "" + temp;
      //Serial.print(temp);
      */
    }
    
    myFile.close();
    /*
    String currLine = "";
    while (!currLine.equals("endPrint"))
    {
      currLine = fullFile.substring(0, fullFile.indexOf("\n"));
      fullFile = fullFile.substring(fullFile.indexOf("\n") + 1);
      Serial.println(currLine);
      cmdReciever(currLine);
    }
    */
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

void adminCmdReciever(String dirtyCmd) {
  String args[2];
  String cmd;
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
  else if (cmd.equals("movTo"))
    moveAdmin(args[0], args[1]);
  else if (cmd.equals("printInfo"))
    adminPrintInfo();
  else if (cmd.equals("printLoc"))
    adminPrintLoc();
  else if (cmd.equals("read"))
    adminRead();
  else if (cmd.equals("signature"))
    adminSig();
  else if (cmd.equals("x.write"))
    adminWrite("x", args[0]);
  else if (cmd.equals("y.write"))
    adminWrite("y", args[0]);
  else if (cmd.equals("plug.write"))
    adminWrite("plug", args[0]);
  else if (cmd.equals("about"))
    about();
  else if (cmd.equals("logoff"))
    goodBye(19);
}

void moveAdmin(String xLocS, String yLocS) {
  int xLoc = xLocS.toInt();
  int yLoc = yLocS.toInt();
  
  if (move(xLoc, yLoc))
    Serial.println("OK");
  else
    Serial.println("Error - beyond grid");
}

void adminPrintInfo() {
  Serial.println("Location:");
  Serial.println("Supposed to be: (" + String(currX) + "," + String(currY) + ")");
  Serial.println("Is: (" + String(xAxisServo.read()/degreesPerX) + "," + String(yAxisServo.read()/degreesPerY) + ")");
  Serial.println("#########");
  Serial.println("Servo motor degrees:");
  Serial.println("X axis readings: " + String(xAxisServo.read()));
  Serial.println("Y axis readings: " + String(yAxisServo.read()));
  Serial.println("#########");
  Serial.println("Current plug readings: " + String(plugServo.read()));
  Serial.println("Plug open degrees: " + String(90));
  Serial.println("Plug close degrees: " + String(0));
}

void adminPrintLoc() {
  Serial.println("Supposed to be: (" + String(currX) + "," + String(currY) + ")");
  Serial.println("Is: (" + String(xAxisServo.read()/degreesPerX) + "," + String(yAxisServo.read()/degreesPerY) + ")");
}

void adminRead() {
  Serial.println("Servo motor degrees:");
  Serial.println("X axis readings: " + String(xAxisServo.read()));
  Serial.println("Y axis readings: " + String(yAxisServo.read()));
  Serial.println("#########");
  Serial.println("Current plug readings: " + String(plugServo.read()));
  Serial.println("Plug open degrees: " + String(90));
  Serial.println("Plug close degrees: " + String(0));
}

void adminSig() {
  Serial.println("/---------------\\");
  Serial.println("| A             |");
  Serial.println("|  /---------\\  |");
  Serial.println("|   |  |  / |   |");
  Serial.println("|   |  | /  |   |");
  Serial.println("|   |  |/   |   |");
  Serial.println("|   |  |\\   |   |");
  Serial.println("|   |  | \\  |   |");
  Serial.println("|   |  |  \\ |   |");
  Serial.println("|  \\---------/  |");
  Serial.println("|              A|");
  Serial.println("\\---------------/");
}

void adminWrite(String motor, String degS) {
  float deg = degS.toFloat();
  if (motor.equals("x"))
  {
    xAxisServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to X motor");
  }
  else if (motor.equals("y"))
  {
    yAxisServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to Y motor");
  }
  else if (motor.equals("plug"))
  {
    plugServo.write(deg);
    Serial.println("Wrote " + String(deg) + " degrees to Plug motor");
  }
}

void about() {
  Serial.println("This is the arduino-side\nsoftware for the PanPrinter machine");
  Serial.println("Last Update: 28/02/2016");
  Serial.println("Programmer: OS");
}
