#ifndef _K_SENSOR_H_
#define _K_SENSOR_H_

#define K_SENSOR_TYPE_NONE 1000
#define K_SENSOR_TYPE_PROXIMITY 1001
#define K_SENSOR_TYPE_TEMPHUM 1002

#define K_OUTPUT_TYPE_NONE 2000
#define K_OUTPUT_TYPE_HTTP 2001

#define K_DATAKEY_NONE 3000
#define K_DATAKEY_TEMPERATURE 3001
#define K_DATANAME_TEMPERATURE "Temperature"
#define K_DATAKEY_HUMIDITY 3002
#define K_DATANAME_HUMIDITY "Humidity"
#define K_DATAKEY_TEMPHUM 3003
#define K_DATANAME_TEMPHUM "Temp_Hum"
#define K_STATUS_OK 0

typedef int k_sensor_type;
typedef int k_output_type;

typedef int k_datakey;
typedef float k_dataval;
typedef int k_status;

#endif // _K_SENSOR_H_
