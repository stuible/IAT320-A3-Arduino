#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <SoftwareSerial.h>
SoftwareSerial BTserial(0, 1); // RX | TX

Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

//char dimensions[3] = {'x', 'y', 'z'};

int historyIndex = 0;
float accX[5] = {0, 0, 0, 0, 0};
float accY[5] = {0, 0, 0, 0, 0};
float accZ[5] = {0, 0, 0, 0, 0};

float accPrevX[5] = {0, 0, 0, 0, 0};
float accPrevY[5] = {0, 0, 0, 0, 0};
float accPrevZ[5] = {0, 0, 0, 0, 0};
float accThreshold = 10;
float yThreshold = 4;

int yMovementIndex = 0;
bool yMovementBegun = false;
float moveY[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

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

  /* Enable auto-gain */
  mag.enableAutoRange(true);

  /* Initialise the sensor */
  if (!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 Magnometer detected ... Check your wiring!");
    while (1);
  }

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
  sensors_event_t magEvent;
  mag.getEvent(&magEvent);

  sensors_event_t accEvent;
  accel.getEvent(&accEvent);

  String raw = (String)"{" +
               "mag:{" +
               "x:" + magEvent.magnetic.x + "," +
               "y:" + magEvent.magnetic.y + "," +
               "z:" + magEvent.magnetic.z +
               + "}," +
               "acc:{" +
               "x:" + accEvent.acceleration.x + "," +
               "y:" + accEvent.acceleration.y + "," +
               "z:" + accEvent.acceleration.z +
               + "}" +
               + "}\n";


//      Serial.print(raw);
  //  if (accEvent.acceleration.y > 10) Serial.println(accEvent.acceleration.y);

  //  Serial.println("Abotu to do the thing");

  // Transfer current current data into another array so it can be compared after the current array once full
  accPrevX[historyIndex] = accX[historyIndex];
  accPrevY[historyIndex] = accY[historyIndex];
  accPrevZ[historyIndex] = accZ[historyIndex];

  // Update current index with current acceleration data
  accX[historyIndex] = accEvent.acceleration.x;
  accY[historyIndex] = accEvent.acceleration.y;
  accZ[historyIndex] = accEvent.acceleration.z;

  if (historyIndex <  3) historyIndex++;
  // Run every time the arrays have been filled
  else {
    String json = ""; // String that will contain json data for later trandsport over bluetooth
    historyIndex = 0; // Reset history index

    float xPrevAvg = arrayAverage(accPrevX, 5);
    float yPrevAvg = arrayAverage(accPrevY, 5);
    float zPrevAvg = arrayAverage(accPrevZ, 5);

    float xAvg = arrayAverage(accX, 5);
    float yAvg = arrayAverage(accY, 5);
    float zAvg = arrayAverage(accZ, 5);

    //Detect a forward (Y) Motion when the sensor is upright
    if (yAvg > yThreshold && zAvg > 9) {
      if (!yMovementBegun) {
        yMovementBegun = true;
        moveY[yMovementIndex] = yAvg;
        yMovementIndex++;
        Serial.println("Begin Y Movement");
      }
      else {
        Serial.println("----- Y Movement");
        if (yMovementIndex < 5) moveY[yMovementIndex] = yAvg;
        yMovementIndex++;
      }
    }
    else  {
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

        if (yMovementIndex < 5 && yMovAvg > 0.5 && yMovAvg < 10) {
//          Serial.println("DAB");
          json += "action: \"dab\",";
        }
//        if (yMovementIndex < 5 && yMovAvg > 0.5) {
//          Serial.println("ALMOST BUT ur ymovAvg is too high");
//        }
        else if (yMovementIndex >= 3 && yMovAvg > 10) {
//          Serial.println("PUNCH");
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


    //Detect Non graviational movement
    float nonGravAcc = (max((abs(xAvg) +  abs(yAvg) + abs(zAvg)) - 15.5, 0)); //Just a half-assed not very scientific formula I came up with that mostly removes gravity
    if (nonGravAcc != 0 && !yMovementBegun) {
//      Serial.println(nonGravAcc);

      // If the last movement was within a second of this one AND our array isn't full
      if (millis() - generalMovementLastMillis < 1000) {
//        Serial.println("Related movement");

        moveGeneral[generalMovementIndex] = nonGravAcc;

        generalMovementIndex++;
      }
      else {
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

    //    if (xAvg - xPrevAvg > 5){ json += "acc:{qmove: ['x']}"; Serial.println("Moved X Quickly!"); }
    //    if (yAvg - yPrevAvg > 5) Serial.println("Moved Y Quickly!");
    //    if (zAvg - zPrevAvg > 2){ json += "acc:{qmove: ['z']}"; Serial.println("Moved Up Quickly!"); }
    //    else { json += "acc:{qmove: ['none']}"; Serial.println("Stayed Still"); }
    //    if(xAvg - xPrevAvg < 5 && yAvg - yPrevAvg < 5 && zAvg - zPrevAvg < 2){ json += "acc:{qmove: ['none']}"; Serial.println("Stayed Still"); }

    //        json += (String)"mag:{" +
    //                      "x:" + magEvent.magnetic.x + "," +
    //                      "y:" + magEvent.magnetic.y + "," +
    //                      "z:" + magEvent.magnetic.z +
    //                      + "}," +
    //                      "acc:{" +
    //                      "x:" + accEvent.acceleration.x + "," +
    //                      "y:" + accEvent.acceleration.y + "," +
    //                      "z:" + accEvent.acceleration.z +
    //                      + "}";
    if (json.length() != 0) Serial.print("{" + json + "}\n");

    if (json.length() != 0) BTserial.print("{" + json + "}\n");
  }



  //  //Right Way Up Detector
  //  if (zAvg > 8 && yAvg < 5 && xAvg < 5) Serial.println("Right Way Up");
  //  else Serial.println("Wrong Way!");


  //  Serial.println(arrayAverage(accZ, 10));

  //  if(accEvent.acceleration.x > accThreshold) {Serial.print("x: "); Serial.println(accEvent.acceleration.x); }
  //  if(accEvent.acceleration.y > accThreshold) {Serial.print("y: "); Serial.println(accEvent.acceleration.y); }
  //  if(accEvent.acceleration.z > accThreshold) {Serial.print("z: "); Serial.println(accEvent.acceleration.z); }


  //  for (int i = 0; i < sizeof(dimensions); i++) {
  //
  //  }

  //  delay(50)
}
