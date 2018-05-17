#include <Servo.h>

Servo xRightAxisServo;
Servo xLeftAxisServo;
Servo yAxisServo;

int xRightAxisServoPin = 10;
int xLeftAxisServoPin = 11;
int yAxisServoPin = 12;

void setup() {
  Serial.begin(9600);
  xRightAxisServo.attach(xRightAxisServoPin);
  xLeftAxisServo.attach(xLeftAxisServoPin);
  yAxisServo.attach(yAxisServoPin);
}

void loop() {
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
  if (cmd.equals("checkCon"))
    conChecker(args[0]);
  else if (cmd.equals("printInfo"))
    adminPrintInfo();
  else if (cmd.equals("read"))
    adminRead();
  else if (cmd.equals("x.write"))
    adminWrite("x", args[0]);
  else if (cmd.equals("xR.write"))
    adminWrite("xR", args[0]);
  else if (cmd.equals("xL.write"))
    adminWrite("xL", args[0]);
  else if (cmd.equals("y.write"))
    adminWrite("y", args[0]);
}

void adminPrintInfo() {
  Serial.println("Servo motor degrees:");
  Serial.println("X axis (R) readings: " + String(xRightAxisServo.read()));
  Serial.println("X axis (L) readings: " + String(xLeftAxisServo.read()));
  Serial.println("Y axis readings: " + String(yAxisServo.read()));
}

void adminRead() {
  Serial.println("Servo motor degrees:");
  Serial.println("X axis (R) readings: " + String(xRightAxisServo.read()));
  Serial.println("X axis (L) readings: " + String(xLeftAxisServo.read()));
  Serial.println("Y axis readings: " + String(yAxisServo.read()));
}

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

void conChecker(String confS) {
  int conf = confS.toInt();
  Serial.println("OK " + String(conf) + " received");
}
