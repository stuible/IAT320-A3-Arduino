#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <SoftwareSerial.h>
SoftwareSerial BTserial(0, 1); // RX | TX

// Taken From Adafruit LSM303DLHC Acceleromter Example Code
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

//Index and arrays to store historical acc. data
int historyIndex = 0;
float accX[4] = {0, 0, 0, 0};
float accY[4] = {0, 0, 0, 0};
float accZ[4] = {0, 0, 0, 0};

float accThreshold = 10;
float yThreshold = 4;

//Index and arrays to store Y data for gestures
int yMovementIndex = 0;
bool yMovementBegun = false;
float moveY[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

//Index and arrays to store general movement data
int generalMovementIndex = 0;
int generalMovementBegun = false;
float moveGeneral[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
double long generalMovementLastMillis = 0;
bool sentGeneralMovement = true;
float lastGeneralMovement = 0;

void setup(void)
{
  //#ifndef ESP8266
  //  while (!Serial);     // will pause until serial console opens
  //#endif
  Serial.begin(9600);
  BTserial.begin(9600);
  Serial.println("Begin"); Serial.println("");

  if (!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 Accelerometre detected ... Check your wiring!");
    while (1);
  }


  /* Display some basic information on this sensor */
  displaySensorDetails();
}

void loop(void)
{
  /* Get a new sensor event */
  sensors_event_t accEvent;
  accel.getEvent(&accEvent);

  //Create raw debug JSON acc data
  String raw = (String)"{" +
               "acc:{" +
               "x:" + accEvent.acceleration.x + "," +
               "y:" + accEvent.acceleration.y + "," +
               "z:" + accEvent.acceleration.z +
               + "}" +
               + "}\n";


//      Serial.print(raw);


  // Update current index with current acceleration data
  accX[historyIndex] = accEvent.acceleration.x;
  accY[historyIndex] = accEvent.acceleration.y;
  accZ[historyIndex] = accEvent.acceleration.z;

  // If array is not yet full, incremement history index and carry  on
  if (historyIndex <  2) historyIndex++;
  // Run every time the arrays have been filled
  else {
    String json = ""; // String that will contain json data for later trandsport over bluetooth
    historyIndex = 0; // Reset history index

    float xAvg = arrayAverage(accX, 4);
    float yAvg = arrayAverage(accY, 4);
    float zAvg = arrayAverage(accZ, 4);

    //Detect a forward (Y) Motion when the sensor is upright (ish)
    if (yAvg > yThreshold && zAvg > 9) {
      //Movement has begun
      if (!yMovementBegun) {
        yMovementBegun = true;
        moveY[yMovementIndex] = yAvg;
        yMovementIndex++;
        Serial.println("Begin Y Movement");
      }
      //Movement is happening
      else {
        Serial.println("----- Y Movement");
        if (yMovementIndex < 5) moveY[yMovementIndex] = yAvg;
        yMovementIndex++;
      }
    }
    else  {
      //Movement is Finished
      if (yMovementBegun) {
        Serial.println("Done Y Movement");
        
        float yMovAvg = arrayAverageNoZeros(moveY, 5);

        Serial.print("yMovAvg: "); Serial.println(yMovAvg);
        Serial.print("yMovementIndex: "); Serial.println(yMovementIndex);

        //On Average, the force of the movement
        //        Serial.print("Avg Movement Force: ");
        //        Serial.println(yMovAvg);

        //How long the motion took
        //        Serial.print("Movement Time: ");
        //        Serial.println(yMovementIndex);

        // Dab detection
        if (yMovementIndex < 4 && yMovAvg > 0.5 && yMovAvg < 9) {
          json += "action: \"dab\",";
        }
        // Punch Detection
        else if (yMovementIndex >= 3 && yMovAvg >= 9) {
          json += "action: \"punch\" ,";
        }

        //Reset Variables
        for (int i = 0; i < 10; i++) {
          moveY[i] = 0;
        }
        yMovementIndex = 0;
      }
//      else Serial.println("Movement was below threshold");
      yMovementBegun = false;
    }


    /* Detect Non Graviational Movement:

    Just a half-assed, not very scientific formula I came up with that mostly removes 
    gravity and only shows additional motion (in any direction).
    */
    float nonGravAcc = (max((abs(xAvg) +  abs(yAvg) + abs(zAvg)) - 14, 0)); 
    if (nonGravAcc != 0 && !yMovementBegun) {
//      Serial.println(nonGravAcc);

      // If the last movement was within a second of this one AND our array isn't full then the movement is a continuation of the last
      if (millis() - generalMovementLastMillis < 1000) {
//        Serial.println("Related movement");

        moveGeneral[generalMovementIndex] = nonGravAcc;

        generalMovementIndex++;
      }
      else {
          // This movement is unrelated to the last one
//        Serial.println("UnRelated movement");
      }

      generalMovementLastMillis = millis();
    }
    // If it's been more than 1 second without movement, take what we have and use that 
    else if (millis() - generalMovementLastMillis > 1000 && generalMovementIndex > 0) {
      Serial.println("Movement Timeout");
      generalMovementIndex = 0;
      sentGeneralMovement = false;
    }

    //If general movement array is full, send average
    if (generalMovementIndex >= 9) {
      generalMovementIndex = 0;
      sentGeneralMovement = false;
    }

    lastGeneralMovement = nonGravAcc;

    // If current array average hasn't been set and index has been set back to zero, send the average
    if (generalMovementIndex == 0 && !sentGeneralMovement) {
      float avgMoveGeneral = arrayAverage(moveGeneral, 10);
//      Serial.print("Genreal Movement Average: ");
//      Serial.println(avgMoveGeneral);
      json += "gMov: " + (String) avgMoveGeneral + ",";
      sentGeneralMovement = true;
    }

    if (json.length() != 0) Serial.print("{" + json + "}\n");

    if (json.length() != 0) BTserial.print("{" + json + "}\n");
  }

  //  delay(50)
}
