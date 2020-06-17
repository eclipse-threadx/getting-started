#ifndef SENSOR_H
#define SENSOR_H

void lps22hb_config(void);
float lps22hb_data_read(void);


typedef struct {
  float humidity_perc;
  float temperature_degC;
} hts221_data_t;

void hts221_config(void);
hts221_data_t hts221_data_read(void);

#endif