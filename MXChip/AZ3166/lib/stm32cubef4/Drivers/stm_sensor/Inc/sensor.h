#ifndef SENSOR_H
#define SENSOR_H

typedef enum 
{
  SENSOR_OK = 0,
  SENSOR_ERROR = 1,
  //SENSOR_TIMEOUT = 2
} Sensor_StatusTypeDef;

typedef struct
{
    float pressure_hPa;
    float temperature_degC;
}lps22hb_t;

Sensor_StatusTypeDef lps22hb_config(void);
lps22hb_t lps22hb_data_read(void);


typedef struct {
  float humidity_perc;
  float temperature_degC;
} hts221_data_t;

Sensor_StatusTypeDef hts221_config(void);
hts221_data_t hts221_data_read(void);

typedef struct { 
  float acceleration_mg[3];
 float angular_rate_mdps[3];
 float temperature_degC;
}lsm6dsl_data_t;

Sensor_StatusTypeDef lsm6dsl_config(void);
lsm6dsl_data_t lsm6dsl_data_read(void);

typedef struct {
  float magnetic_mG[3];
  float temperature_degC;
} lis2mdl_data_t;

Sensor_StatusTypeDef lis2mdl_config(void);
lis2mdl_data_t lis2mdl_data_read(void);

#endif