#include <QTRSensors.h>
  
  //Notes Pt 2: 1) For infinite loop, follow right edge if current desiredPos leads to infinite loop!!!
  // Don't measure POT!! You set Kp to 0!!
  // Base Speed is good for bends
  // Straight line speed needs increase
  // Old bootloader? Could that be causing speed issues?
  
  
  const int PWMA = 11; // Pololu drive A
  const int AIN2 = 10;
  const int AIN1 = 9;
  const int STDBY = 8;
  const int BIN1 = 7; // Pololu drive B
  const int BIN2 = 6;
  const int PWMB = 5;
  int sensorL = 5;
  int sensorM = 6;
  int sensorR  = 7;
  unsigned int sensorValues[3];
  int rightVal;
  int midVal;
  int leftVal;
  int baseL = 255; //230 was FAST!, so was 230   //adjust during testing
  int baseR = 255;   //adjust during testing
  int speedL;
  int speedR;
  float Kp = 1;//0.5; //1
  float Kd = 5;//10; //needs work //0.01 good, changed to 10 to adjust for lastError - error change, 4:07 PM Friday, try lowering a tad (or increase, but just a smidge).
  float Ki = 0; //what could this do?
  unsigned int pos;
  int error;
  float time;
  float elapsedTime;
  float timePrev;
  int desiredPos = 1000; //1500 //directly below sensor 1 (mid), try 0 for below sensor 0 (left) for right 
  int lastError;
  int maxSpeed = 255; //adjust during testing 
  int led = 13;
  float PID;
  int count = 1;
  int t;
  
  QTRSensorsAnalog qtra((unsigned char[]) {7, 6, 5}, 3); // create object
  
  void setup()
  {
  
    Serial.begin(9600);
  
    pinMode(PWMA, OUTPUT);
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
    pinMode(PWMB, OUTPUT);
    pinMode(STDBY, OUTPUT);
    pinMode(A7, OUTPUT);
    digitalWrite(STDBY, HIGH);

    //5 second calibration
    digitalWrite(led, HIGH);  
    for (int i = 0; i < 250; i++)
    {
      qtra.calibrate();
      delay(20);
    }
    digitalWrite(led, LOW); 
  }
  
  
  void loop()
  {
      
    //Switch Case All White vs. Black Light, make a counter for the three white obstacles

    leftVal = analogRead(sensorL);
    rightVal = analogRead(sensorR);
    midVal = analogRead(sensorM);
  
    if (rightVal < 300 && midVal < 300 && leftVal < 300){
     if (count == 1){
      //Narrow part, drive straight
       drive(255, 255); //255, 244 //perfect!...almost
       delay(2880); //2850//3200//1300
       count += 1;
      }
  
    else if (count == 2){
        //SMALL RADIUS TURN, decrease that time?
        t = millis();
        while(millis() - t < 3000){   //Set while loop period to 3000 ms
  
          unsigned int pos = qtra.readLine(sensorValues);
          error = pos - desiredPos;
          PID =  3* error + 1.5 * (error - lastError); //Really big? Maybe try diving by time again!!!!!!!!!!
          lastError = error;
          speedL = 250 - PID;
          speedR = 250 + PID;
          speedL = constrain(speedL, speedL, maxSpeed);
          speedR = constrain(speedR, speedR, maxSpeed);
          drive(speedL, speedR);
        }
        count += 1;
      }
    else if (count == 3){
      //Go forward for some time
      //Turn after that time to the left
      drive(255, 255); //forward
      //delay(1500);
      delay(1850); //1600//changed so that rover doesn't get stuck
      drive(-255, 255); //turn left 1) -255 might have to be 0, 2) Try adding some left speed to more covered
      delay(700);//1000 // might be able to reduce
      count += 1;
    }
    else if (count == 4){
      //Make right turn
      drive(255, -255); //turn right -255 might be 0
      delay(500); //1000
      count += 1;
    }
    else if(count == 5){ //could be parameters, speed, etc.., make this shittier!! 1) Take out Kd, 2) reduce Kd, or increase Kd, 3) slow it down
      //1) Make right-edge follower
      t = millis();
       while(millis() - t < 6000){ //Set while loop period to 6000 ms
  
         unsigned int pos = qtra.readLine(sensorValues);
         error = pos - 1500;
         PID =  1.7* error; //1.3 * (error - lastError); //Kp < 1;
         lastError = error;
         speedL = 250 - PID; //255
         speedR = 250 + PID; //255
         speedL = constrain(speedL, speedL, maxSpeed);
         speedR = constrain(speedR, speedR, maxSpeed);
         drive(speedL, speedR);
        }     
    }  
    }
  
    unsigned int pos = qtra.readLine(sensorValues);
    error = pos - desiredPos;
    PID = Kp * error + Kd * (error - lastError);
    lastError = error;
    speedL = baseL - PID;
    speedR = baseR + PID;

    /*
       if (speedL > maxSpeed)
        speedL = maxSpeed;
       if (speedR > maxSpeed)
        speedR = maxSpeed;
       if (speedL < 0)
        speedL = 0;
       if (speedR < 0)
        speedR = 0;
        */
     
    speedL = constrain(speedL, speedL, maxSpeed);
    speedR = constrain(speedR, speedR, maxSpeed);

    drive(speedL, speedR);
  
  }
  
  void motorWrite(int spd, int pin_IN1 , int pin_IN2 , int pin_PWM)
  {
    if (spd < 0)
    {
      digitalWrite(pin_IN1 , HIGH); // go one way, possibly use DPM
      digitalWrite(pin_IN2 , LOW);
    }
    else
    {
      digitalWrite(pin_IN1 , LOW); // go the other way, possibly use DPM
      digitalWrite(pin_IN2 , HIGH);
    }
    analogWrite(pin_PWM , abs(spd)); 
  }
  
  
  void drive(int speedL, int speedR)
  {
    motorWrite(-1 * speedL, AIN1, AIN2, PWMA);
    motorWrite(-1 * speedR, BIN1, BIN2, PWMB);
  }
