#ifndef _K_BASE_CLASS_H_
#define _K_BASE_CLASS_H_

// Enable light sleep: https://efcomputer.net.au/blog/esp8266-light-sleep-mode/
extern "C" {
   #include "gpio.h"
   #include "user_interface.h"
}

#include "k_persistent.h"

// Default sensor is none (=toggler)
class BaseSensor
{
  private:
  protected:
    k_sensor_type _sensor_type;
    k_status _status;
    const char* _sensor_name;
    const char* _key_name;

  public:
    BaseSensor(): _sensor_type(K_SENSOR_TYPE_NONE), _status(0), _sensor_name("Base Sensor"), _key_name("BaseKey") {}
    BaseSensor(const char* sensor_name, const char* key_name):_sensor_name(sensor_name), _key_name(key_name){}
    ~BaseSensor() {}
    virtual k_dataval read(k_datakey key)
    {
      _status = !_status;
      return _status;
    }
};

#if(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_NONE)
typedef BaseSensor Sensor;
#elif(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT11 || K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT22)
#include <dht.h>
#define dht_apin 5

class TempHumSensor: public BaseSensor
{
  private:
    dht _dht;

  public:
    TempHumSensor(): BaseSensor("TempHum Sensor","TempHum") {
      //_dht = new DHT();
    }

    k_dataval read(k_datakey key)
    {
#if(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT11)
      _dht.read11(dht_apin);
#elif(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT22)
      _dht.read22(dht_apin);
#endif
      switch (key)
      {
        case K_DATAKEY_TEMPERATURE:
          return _dht.temperature;
        case K_DATAKEY_HUMIDITY:
          return _dht.humidity;
        case K_DATAKEY_TEMPHUM:
#if(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT11)
          return _dht.temperature*100+_dht.humidity*1;
#elif(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT22)
          return (_dht.temperature+40)*10000+_dht.humidity*10;
#endif
      }
      return K_STATUS_OK;
    }
};
typedef TempHumSensor Sensor;
#elif(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_PMS7003)
#import "PMS.h"

class PMSensor: public BaseSensor
{
  private:
    PMS _pms;

  public:
    PMSensor(): BaseSensor("PM Sensor","PM") {
    }

    k_dataval read(k_datakey key)
    {
      _dht.read11(dht_apin);
      switch (key)
      {
        case K_DATAKEY_PMS7003:
          return _dht.temperature;
        case K_DATAKEY_HUMIDITY:
          return _dht.humidity;
        case K_DATAKEY_TEMPHUM:
#if(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT11)
          return _dht.temperature*100+_dht.humidity*1;
#elif(K_SENSOR_TYPE_TO_USE==K_SENSOR_TYPE_DHT22)
          return (_dht.temperature+40)*10000+_dht.humidity*10;
#endif
      }
      return K_STATUS_OK;
    }
};
typedef PMSensor Sensor;
#endif

// Default ouptut is LED
class BaseOutput
{
  protected:
    k_status _status;

  public:
    BaseOutput(): _status(0)
    {
      //pinMode(LED_BUILTIN, OUTPUT);
      Serial.begin(9600);
    }
    ~BaseOutput() {}
    virtual void printMACAddress(){}
    virtual k_status write(k_datakey key, k_dataval val)
    {
      Serial.println("BaseOutput");
      digitalWrite(LED_BUILTIN, HIGH);
      delay(10);
      digitalWrite(LED_BUILTIN, LOW);
    }
};
#if(K_OUTPUT_TYPE_TO_USE==K_OUTPUT_TYPE_NONE)
typedef BaseOutput Output;
#elif(K_OUTPUT_TYPE_TO_USE==K_OUTPUT_TYPE_HTTP)
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
extern "C" {
  //#include "user_interface.h"
  //#include "wifi_connection_info.h"
}
    KPersistentRTC _kp;

class HTTPOutput: BaseOutput
{
  private:
    String wMACAddress = WiFi.macAddress();
    String serverURL = WIFI_SERVER_URL;
    String serverCertFingerprint = WIFI_SERVER_CERT_FP;
    int count;
    int max_count;
    float freq_in_second;
    String catStr;
    int _isAsleep;
    //KPersistentRTC _kp;
    //KPersistentLittleFS _kp;

    k_status connect()
    {
      //Serial.begin(115200);                 //Serial connection
      WiFi.mode(WIFI_STA);
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);   //WiFi connection
    
      // https://techtutorialsx.com/2017/04/09/esp8266-get-mac-address/
      Serial.println();
      Serial.print("MAC: ");
      Serial.println(wMACAddress);
    
      while (WiFi.status() != WL_CONNECTED) {  //Wait for the WiFI connection completion
        delay(500);
        Serial.print("Waiting...");
      }
    
      Serial.println("Connected");
      //this->connected();
    }


    int wifi_get(const char* url)
    {//Check WiFi connection status
      if (WiFi.status() != WL_CONNECTED)
      {
        this->connect();
      }
      this->wifi_wakeup();
 
      { 
        HTTPClient http;    //Declare object of class HTTPClient

        // https://techtutorialsx.com/2016/07/17/esp8266-http-get-requests/
        http.begin(url, serverCertFingerprint.c_str());
        int httpCode = http.GET(); //Send the request
Serial.println(url);
        if (httpCode > 0) { //Check the returning code
          String payload = http.getString();   //Get the request response payload
          Serial.println(payload);                     //Print the response payload
        }
        else
        {
          String payload = http.getString();   //Get the request response payload
          Serial.println(String("Return:")+payload);                     //Print the response payload
        }

        http.end();   //Close connection
      }
      return K_STATUS_OK;
    }

  public:
    HTTPOutput():count(0),catStr(""),max_count(60), freq_in_second(20), _isAsleep(0)
    {
      wMACAddress.replace(":", "_");
      this->printMACAddress();

      _kp.init(61*sizeof(int));
    }
    ~HTTPOutput() {}

    void printMACAddress(){      Serial.println(wMACAddress);}

    float getFreqInSecond()const {return freq_in_second;}
    void setFreqInSecond(float f){freq_in_second=f;}
    void setMaxCount(int c){max_count=c;}

    k_status write_to_server(const char* message)
    {
      return wifi_get((serverURL + "?key=" + wMACAddress + "&"+message).c_str());
    }

    k_status write_to_wifi_rtc(const char* fmt, int val)
    {
      int rtc_count;
      // Read counter
      //system_rtc_mem_read(RTCMEMORYSTART, &rtc_count, sizeof(rtc_count));
      _kp.read(0, &rtc_count, sizeof(rtc_count));
Serial.println(String("count: ")+rtc_count);

      
      if (rtc_count<0 || rtc_count>this->max_count) rtc_count = 0;
      count = rtc_count;

      // Write new val to RTC memory
      //system_rtc_mem_write(RTCMEMORYSTART+sizeof(rtc_count)+sizeof(val)*count, &val, sizeof(val));
      _kp.write(sizeof(rtc_count)+sizeof(val)*count, &val, sizeof(val));

      //count: number of values in RTC
      count++;
        
      if(count==max_count)
      {
        // Submit to wifi
        catStr=String("");
        for(int ii = 0; ii<max_count; ++ii)
        {
          int rtc_val;
          catStr+=",";
          // Real all recorded values
          //system_rtc_mem_read(RTCMEMORYSTART+sizeof(rtc_count)+sizeof(rtc_val)*ii, &rtc_val, sizeof(rtc_val));
          _kp.read(sizeof(rtc_count)+sizeof(rtc_val)*ii, &rtc_val, sizeof(rtc_val));
          catStr+=rtc_val;
        }
        write_to_server((String("value=")+fmt+":"+catStr).c_str());
        WiFi.mode( WIFI_OFF );
        
        // Set counter to 0
        count=0;
      }
      rtc_count = count;
      // Update counter
      //system_rtc_mem_write(RTCMEMORYSTART, &rtc_count, sizeof(rtc_count));
      _kp.write(0, &rtc_count, sizeof(rtc_count));
Serial.println(String("writing count: ")+rtc_count);
      _kp.read(0, &rtc_count, sizeof(rtc_count));
Serial.println(String("wrote count: ")+rtc_count);

      ESP.deepSleep((int)(freq_in_second*1000*1000));
      delay(1);
    }

    k_status write_to_wifi(const char* fmt, int val)
    {
      Serial.println(String(fmt)+":"+val);

      if(count==max_count)
      {
        WiFi.forceSleepWake();          
        delay(1);
        write_to_server((String("value=")+fmt+":"+catStr).c_str());
        // Modem sleep
        WiFi.mode( WIFI_OFF );
        WiFi.forceSleepBegin();
        delay(1);
        
        catStr="";
        count=0;
      }
      else
      {
        catStr+=",";
        catStr+=(int)val;
        count++;
Serial.println(String("count: ")+count);
      }
    }

    k_status write(k_datakey key, k_dataval val) 
    {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(10);
      digitalWrite(LED_BUILTIN, LOW);

      if(key==K_DATAKEY_TEMPHUM)
      {
        //write_to_wifi("TH", val);
        write_to_wifi_rtc("TH", val);
      }
      else if(key==K_DATAKEY_TEMPHUM_DHT22)
      {
        //write_to_wifi("TH2", val);
        write_to_wifi_rtc("TH2", val);
      }
      return K_STATUS_OK;
    }

    k_status connected()
    {
      return write_to_server("value=connected");
    }

    String getMACAddress()const {return wMACAddress;}

    void wifi_sleep()
    {
      if(!this->_isAsleep)
      {
        WiFi.mode( WIFI_OFF );
        WiFi.forceSleepBegin();
        delay(1);
        this->_isAsleep = 1;
      }
    }
    void wifi_wakeup()
    {
      if(this->_isAsleep)
      {
        WiFi.forceSleepWake();
        delay(1);
        this->_isAsleep = 0;
      }
    }

};
typedef HTTPOutput Output;
#endif

#endif// _K_BASE_CLASS_H_
