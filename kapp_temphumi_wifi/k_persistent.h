#include "Arduino.h"
#include "LittleFS.h"
// Using littlefs: https://www.mischianti.org/2020/06/22/wemos-d1-mini-esp8266-integrated-littlefs-filesystem-part-5/
extern "C" {
   #include "user_interface.h"
}

typedef unsigned int size_t;

class KPersistent
{
  public:
  KPersistent()
  {}
  virtual void init(size_t max_size=1){}

  virtual bool write(size_t dsk_char_pos, const void* mem, size_t len);

  virtual bool read(size_t dsk_char_pos, void* mem, size_t len);
   
  private:
  
};



class KPersistentRTC: public KPersistent
{
  public:
  KPersistentRTC():_rtc_mem_start(64){}
  
  virtual void init(size_t max_size=1){
    if(_buffer) delete[] _buffer;
    _buffer_size = max_size;
    _buffer = new char[_buffer_size];
    memset(_buffer, _buffer_size, 0);
  }
  
  bool write(size_t dsk_char_pos, const void* mem, size_t len)
  {
    system_rtc_mem_read(_rtc_mem_start, _buffer, _buffer_size);
    memcpy(_buffer + dsk_char_pos, mem, len);
    system_rtc_mem_write(_rtc_mem_start, _buffer, _buffer_size);
  }

  bool read(size_t dsk_char_pos, void* mem, size_t len)
  {
    system_rtc_mem_read(_rtc_mem_start, _buffer, _buffer_size);
    memcpy(mem, _buffer + dsk_char_pos, len);
  }

  private:
  const size_t _rtc_mem_start;
  size_t _buffer_size;
  char* _buffer;
};



class KPersistentLittleFS: public KPersistent
{
  public:
  KPersistentLittleFS(size_t max_size=0): _max_size(max_size)
  {
    this->init(max_size);
  }

  void init_fs(size_t max_size)
  {
      this->_file = LittleFS.open(F("/_KPersistentLittleFS.txt"), "w");
      this->_file.seek(max_size-1, SeekSet);
      char zero='\0';
      bool ret = this->_file.write(&zero, 1);
      Serial.println(String("init_fs result: ")+ret);
      this->_file.close();
  }

  void init(size_t max_size=1)
  {
    LittleFS.begin();
    if(!LittleFS.exists(F("/_KPersistentLittleFS.txt")))
    {
      init_fs(max_size);
      //Serial.println("new");
    }
    else
    {
      //this->_file = LittleFS.open(F("/_KPersistentLittleFS.txt"), "a+");
      //Serial.println("exists");
    }
  }
  
  bool write(size_t dsk_char_pos, const void* mem, size_t len)
  {
    Serial.println(String("Will write ")+*(int*)mem+String(" at ")+dsk_char_pos);
    _file = LittleFS.open(F("/_KPersistentLittleFS.txt"), "a+");
    _file.seek(dsk_char_pos, SeekSet);
    bool ret = _file.write((const uint8_t *)mem, len);
      Serial.println(String("write result: ")+ret);
    int val;
    _file.seek(dsk_char_pos, SeekSet);
    _file.readBytes((char*)&val, sizeof(val));
    Serial.println(String("Wrote: ")+val);
    _file.close();
    return 0;
  }

  bool read(size_t dsk_char_pos, void* mem, size_t len)
  {
    _file = LittleFS.open(F("/_KPersistentLittleFS.txt"), "r");
    _file.seek(dsk_char_pos, SeekSet);
    _file.readBytes((char*)mem, len);
    _file.close();
    return 0;
  }

  private:
  File _file;
  size_t _max_size;
};
