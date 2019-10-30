#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_LSM303_U.h>
#include <SoftwareSerial.h> 
SoftwareSerial BTserial(0, 1); // RX | TX

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Mag_Unified mag = Adafruit_LSM303_Mag_Unified(12345);

/* Assign a unique ID to this sensor at the same time */
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(54321);

void setup(void)
{
//#ifndef ESP8266
//  while (!Serial);     // will pause Zero, Leonardo, etc until serial console opens
//#endif
  Serial.begin(9600);
  BTserial.begin(9600);
  Serial.println("Begin"); Serial.println("");

  /* Enable auto-gain */
  mag.enableAutoRange(true);

  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 Magnometer detected ... Check your wiring!");
    while(1);
  }

   if(!accel.begin())
  {
    /* There was a problem detecting the LSM303 ... check your connections */
    Serial.println("Ooops, no LSM303 Accelerometre detected ... Check your wiring!");
    while(1);
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

  String json = (String)"{" + 
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
        
                 
//  Serial.print(json);

//  delay(50)
}
