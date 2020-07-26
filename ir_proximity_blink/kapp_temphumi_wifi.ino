#include "k_sensor.h"

#define  K_SENSOR_TYPE_TO_USE  K_SENSOR_TYPE_TEMPHUM
#define  K_OUTPUT_TYPE_TO_USE  K_OUTPUT_TYPE_HTTP

// * k_wifi_secrets.h defines
// - WIFI_SERVER_URL
// - WIFI_SERVER_CERT_FP //https://circuits4you.com/2019/01/10/esp8266-nodemcu-https-secured-get-request/
// - WIFI_SSID
// - WIFI_PASSWORD
#include "k_wifi_secrets.h"

#include "k_baseclass.h"

// Enable light sleep: https://efcomputer.net.au/blog/esp8266-light-sleep-mode/
extern "C" {
   #include "gpio.h"
   #include "user_interface.h"
}

// Main loop
Output* output;
Sensor* sensor;
int t_problem_counter = 0;
int t_problem_count_to_report=10;

void setup()
{
  gpio_init(); // Initilise GPIO pins
  output = new Output();
  sensor = new Sensor();

  //output->connect();
  output->setFreqInSecond(20);
  output->setMaxCount(60);

  t_problem_counter = 0;
  t_problem_count_to_report=10;
}

void loop()
{
  k_dataval val = sensor->read(K_DATAKEY_TEMPHUM);
  output->write(K_DATAKEY_TEMPHUM, val);

  float T = val/100;
  if(T>8.0f && T<18.0f)
  {
    t_problem_counter++;
  }
  if(t_problem_counter>=t_problem_count_to_report)
  {
    output->write_to_server((String("Problem found for ")+t_problem_counter+" times, T="+T).c_str());
    t_problem_counter=0;
  }
  

  //https://github.com/esp8266/Arduino/issues/1381
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T); // set sleep type, the above    posters wifi_set_sleep_type() didnt seem to work for me although it did let me compile and upload with no errors 
  // * Regular sleep
  delay(output->getFreqInSecond()*1000);
  wifi_fpm_set_sleep_type(NONE_SLEEP_T); // set sleep type, the above    posters wifi_set_sleep_type() didnt seem to work for me although it did let me compile and upload with no errors 

  // * Deep sleep uses about 20uA of power. However, it restarts the whole thing. 
  //ESP.deepSleep(output->getFreqInSecond()*1000);

  // * Modem sleep:
}

/*
  // constants won't change. They're used here to set pin numbers:
  const int digitalPin = D4;     // the number of the pushbutton pin
  const int ledPin =  LED_BUILTIN;      // the number of the LED pin

  // variables will change:
  int digitalPinState = 0;         // variable for reading the pushbutton status
  String wMACAddress = WiFi.macAddress();
  String serverURL = WIFI_SERVER_URL;
  String serverCertFingerprint = WIFI_SERVER_CERT_FP;

  enum SensortState {SensorState_unknown, SensorState_open, SensorState_closed};

  SensortState currentState = SensorState_unknown;
  SensortState previousState = SensorState_unknown;
  int sensorStateUnchangedCount = 0;
  int heartBeatCount = 0;

  // Every 3600 counts, send heart beat to server
  const int heartBeatFrequency = 3600;

  // If current state is maintained for minReportCount, submit report
  const int minReportCount = 10;

  void setup() {
  // initialize the LED pin as an output:
  pinMode(ledPin, OUTPUT);
  // initialize the pushbutton pin as an input:
  pinMode(digitalPin, INPUT);
*/

/*
  // https://techtutorialsx.com/2016/07/21/esp8266-post-requests/
  // https://techtutorialsx.com/2016/07/17/esp8266-http-get-requests/
  Serial.begin(115200);                 //Serial connection
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //WiFi connection

  // https://techtutorialsx.com/2017/04/09/esp8266-get-mac-address/
   Serial.println();
   Serial.print("MAC: ");
   wMACAddress.replace(":","_");
   Serial.println(wMACAddress);

  while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
    delay(500);
    Serial.println("Waiting for connection");
  }

  Serial.println("Connected");

  wifi_get((serverURL+"?key="+wMACAddress+"&value=connected").c_str(), serverCertFingerprint.c_str());
  Serial.println("submitted");

  }

  int wifi_get(const char* url, const char* fingerprint)
  {
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status

   HTTPClient http;    //Declare object of class HTTPClient

  // https://techtutorialsx.com/2016/07/17/esp8266-http-get-requests/
    http.begin(url, fingerprint);
    int httpCode = http.GET();                                                                  //Send the request

    if (httpCode > 0) { //Check the returning code

      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload

    }

    http.end();   //Close connection
  }
  else
  {
  Serial.println("Not Connected.");
  }
  return 0;
  }

  void blinkNtimes(int n, int duration_ms)
  {
  for(int ii = 0; ii<n; ++ii)
  {
    digitalWrite(ledPin, LOW);//LOW means on
    delay(duration_ms/2);
    digitalWrite(ledPin, HIGH);// HIGH means offs
    delay(duration_ms/2);
  }
  }
  void loop() {
  // read the state of the pushbutton value:
  digitalPinState = digitalRead(digitalPin);

  previousState = currentState;
  if (digitalPinState == HIGH) {// high: no object
    // turn LED on:
    //digitalWrite(ledPin, LOW);
    blinkNtimes(1,100);
    currentState = SensorState_open;
  } else { // digitalPinState == LOW, low: object present
    // turn LED off:
    //digitalWrite(ledPin, LOW);
    blinkNtimes(2,50);
    currentState = SensorState_closed;
  }
  digitalWrite(ledPin, HIGH);
*/
/*
  if(currentState != previousState)
  {
    sensorStateUnchangedCount = 0;
  }
  sensorStateUnchangedCount++;
  heartBeatCount++;

  if(sensorStateUnchangedCount == minReportCount)
  {
    if (currentState==SensorState_open)
    {
      Serial.println("did_open");
      wifi_get((serverURL+"?key="+wMACAddress+"&value=did_open").c_str(), serverCertFingerprint.c_str());
    }
    else if(currentState==SensorState_closed)
    {
      Serial.println("did_close");
      wifi_get((serverURL+"?key="+wMACAddress+"&value=did_close").c_str(), serverCertFingerprint.c_str());
    }
  }
  if(heartBeatCount%heartBeatFrequency==0)
  {
    if (currentState==SensorState_open)
    {
      Serial.println("is_open");
      wifi_get((serverURL+"?key="+wMACAddress+"&value=is_open").c_str(), serverCertFingerprint.c_str());
    }
    else if(currentState==SensorState_closed)
    {
      Serial.println("is_closed");
      wifi_get((serverURL+"?key="+wMACAddress+"&value=is_closed").c_str(), serverCertFingerprint.c_str());
    }
  }
  delay(900);
  }
*/
