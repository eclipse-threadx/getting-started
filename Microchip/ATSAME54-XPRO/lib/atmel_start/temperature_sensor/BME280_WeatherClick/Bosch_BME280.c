/**
 * \file
 *
 * \brief Bosch BME280 driver
 *
 * Copyright (c) 2016-2018 Microchip Technology Inc. and its subsidiaries.
 *
 * \asf_license_start
 *
 * \page License
 *
 * Subject to your compliance with these terms, you may use Microchip
 * software and any derivatives exclusively with Microchip products.
 * It is your responsibility to comply with third party license terms applicable
 * to your use of third party software (including open source software) that
 * may accompany Microchip software.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE,
 * INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY,
 * AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE
 * LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL
 * LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE
 * SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE
 * POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT
 * ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY
 * RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
 * THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * \asf_license_stop
 *
 */

#include "atmel_start.h"
#include <hal_i2c_m_sync.h>
#include "temperature_sensor.h"
#include "Bosch_BME280.h"


typedef union {

    struct {
        uint8_t im_update : 1;
        uint8_t: 2;
        uint8_t measuring : 1;
        uint8_t: 4;
    };
    uint8_t statusReg;
} bme280_status_t;

typedef union {

    struct {
        uint8_t mode : 2;
        uint8_t osrs_P : 3;
        uint8_t osrs_T : 3;
    };
    uint8_t ctrlMeasReg;
} bme280_ctrl_meas_t;

typedef union {

    struct {
        uint8_t spi3w_en : 1;
        uint8_t filter : 3;
        uint8_t t_sb : 3;
    };
    uint8_t configReg;
} bme280_config_t;

typedef struct {
    uint16_t dig_T1;
    int dig_T2;
    int dig_T3;
    uint16_t dig_P1;
    int dig_P2;
    int dig_P3;
    int dig_P4;
    int dig_P5;
    int dig_P6;
    int dig_P7;
    int dig_P8;
    int dig_P9;
    uint8_t dig_H1;
    int dig_H2;
    uint8_t dig_H3;
    int dig_H4;
    int dig_H5;
    signed char dig_H6;
} bme280_calibration_param_t;

   bme280_calibration_param_t calibParam;
   bme280_config_t bme280_config;
   uint8_t bme280_ctrl_hum;
   bme280_ctrl_meas_t bme280_ctrl_meas;
   bme280_status_t bme280_status;
   long adc_T, adc_H, adc_P, t_fine;
   
static long BME280_compensateTemperature(void);
static uint32_t BME280_compensatePressure(void);
static uint32_t BME280_compensateHumidity(void);


#define DEFAULT_STANDBY_TIME    BME280_STANDBY_HALFMS
#define DEFAULT_FILTER_COEFF    BME280_FILTER_COEFF_OFF
#define DEFAULT_TEMP_OSRS       BME280_OVERSAMP_X1
#define DEFAULT_PRESS_OSRS      BME280_OVERSAMP_X1
#define DEFAULT_HUM_OSRS        BME280_OVERSAMP_X1
#define DEFAULT_SENSOR_MODE     BME280_FORCED_MODE


/** BME280 temperature sensor's interface */
static const struct temperature_sensor_interface bme280_interface = { Weather_getTemperatureDegC };

static struct bme280 BME280_descr;

struct i2c_m_sync_desc *descr;


/**
 * \brief Initialize Bosch weather_click
 */
//int bme280_sensor_init();
int WeatherClick_initialize()
{
  uint8_t deviceid;
  
  i2c_m_sync_enable(&I2C_AT24MAC);
  
  BME280_construct(&BME280_descr.parent, &I2C_AT24MAC);
  deviceid =  BME280_getID(); 
  
  printf("Weather click Device ID =  %d \r\n",deviceid);
  printf("Weather_Initialize... \r\n");
  
  BME280_reset();
     
 // delay_ms(5000);    //Startup delay for BME280 sensor
  for(int i=0;i<200000;i++){}
  
  BME280_readFactoryCalibrationParams();
  
  BME280_setStandbyTime(DEFAULT_STANDBY_TIME);                              
  BME280_setFilterCoefficient(DEFAULT_FILTER_COEFF);                          
  BME280_setOversamplingTemperature(DEFAULT_TEMP_OSRS);                       
  BME280_setOversamplingPressure(DEFAULT_PRESS_OSRS);                        
  BME280_setOversamplingHumidity(DEFAULT_HUM_OSRS);                        
  BME280_setSensorMode(DEFAULT_SENSOR_MODE);
  
  BME280_initializeSensor();
  
  return 0;
}

int WeatherClick_waitforRead(void)
{
    if (DEFAULT_SENSOR_MODE == BME280_FORCED_MODE) {
      BME280_startForcedSensing();
    }
    
    while(BME280_isMeasuring());
    BME280_readMeasurements();
    
    return 0;
}

/**
 * \brief Construct bme280 sensor
 */
struct temperature_sensor* BME280_construct(struct temperature_sensor *const me, void *const io)
{
	temperature_sensor_construct(me, io, &bme280_interface);
        
        descr = (struct i2c_m_sync_desc *)(me->io);
        
	return me;
}


void BME280_readBlock(uint8_t reg_add, uint8_t *buffer, uint8_t len) {
      
    buffer[0] = reg_add;
    buffer[1] = 0;
    i2c_m_sync_set_slaveaddr(descr, BME280_ADDR, I2C_M_SEVEN);
    io_write(&descr->io, buffer, 1);
    delay_ms(100);
    io_read(&descr->io, buffer, len);
        
    return;
}

uint8_t BME280_readByte(uint8_t reg_add, uint8_t *buffer) {
  
    buffer[0] = reg_add;
    buffer[1] = 0;
    i2c_m_sync_set_slaveaddr(descr, BME280_ADDR, I2C_M_SEVEN);
    io_write(&descr->io, buffer, 1);
    io_read(&descr->io, buffer, 1);
        
    return buffer[0];
}

void BME280_writeByte(uint8_t reg_add, uint8_t data) {
  
    uint8_t     buffer[2];
    
    buffer[0] = reg_add;
    buffer[1] = data;
    i2c_m_sync_set_slaveaddr(descr, BME280_ADDR, I2C_M_SEVEN);
    io_write(&descr->io, buffer, 2);
        
    return;
}

uint8_t BME280_getID() {
  
    uint8_t                 buffer[2];
  
    BME280_readByte( BME280_ID_REG, buffer);
      
    return buffer[0];
}

void BME280_reset( ) {
  
    BME280_writeByte(BME280_RESET_REG, BME280_SOFT_RESET);
}

void BME280_setStandbyTime(uint8_t sbtime) {
    bme280_config.t_sb = sbtime;
}

void BME280_setFilterCoefficient(uint8_t coeff) {
    bme280_config.filter = coeff;
}

void BME280_setOversamplingTemperature(uint8_t osrs_t) {
    bme280_ctrl_meas.osrs_T = osrs_t;
}

void BME280_setOversamplingPressure(uint8_t osrs_p) {
    bme280_ctrl_meas.osrs_P = osrs_p;
}

void BME280_setOversamplingHumidity(uint8_t osrs_h) {
    bme280_ctrl_hum = osrs_h;
}

void BME280_setSensorMode(uint8_t mode) {
    bme280_ctrl_meas.mode = mode;
}

void BME280_initializeSensor( const struct temperature_sensor *const me ) {
  
    BME280_writeByte(BME280_CONFIG_REG   , bme280_config.configReg);
    BME280_writeByte(BME280_CTRL_HUM_REG , bme280_ctrl_hum);
    BME280_writeByte(BME280_CTRL_MEAS_REG, bme280_ctrl_meas.ctrlMeasReg);
}

void BME280_startForcedSensing(const struct temperature_sensor *const me) {
  
    bme280_ctrl_meas.mode = BME280_FORCED_MODE;
    
    BME280_writeByte(BME280_CTRL_MEAS_REG, bme280_ctrl_meas.ctrlMeasReg);
}

uint8_t BME280_getStatus( ) {
  
    uint8_t                 buffer[2];
    
    bme280_status.statusReg = BME280_readByte(BME280_STATUS_REG, buffer);        
    return buffer[0];
}

uint8_t BME280_isMeasuring() {
    bme280_status.statusReg = BME280_getStatus();
    return (bme280_status.measuring);
}

void BME280_readFactoryCalibrationParams( const struct temperature_sensor *const me ) {
  
    uint8_t paramBuff[24];
    
    BME280_readBlock(BME280_CALIB_DT1_LSB_REG, paramBuff, 24);
    
    calibParam.dig_T1 = (((uint16_t) paramBuff[1]) << 8) + paramBuff[0];
    calibParam.dig_T2 = (((int) paramBuff[3]) << 8) + paramBuff[2];
    calibParam.dig_T3 = (((int) paramBuff[5]) << 8) + paramBuff[4];
    calibParam.dig_P1 = (((uint16_t) paramBuff[7]) << 8) + paramBuff[6];
    calibParam.dig_P2 = (((int) paramBuff[9]) << 8) + paramBuff[8];
    calibParam.dig_P3 = (((int) paramBuff[11]) << 8) + paramBuff[10];
    calibParam.dig_P4 = (((int) paramBuff[13]) << 8) + paramBuff[12];
    calibParam.dig_P5 = (((int) paramBuff[15]) << 8) + paramBuff[14];
    calibParam.dig_P6 = (((int) paramBuff[17]) << 8) + paramBuff[16];
    calibParam.dig_P7 = (((int) paramBuff[19]) << 8) + paramBuff[18];
    calibParam.dig_P8 = (((int) paramBuff[21]) << 8) + paramBuff[20];
    calibParam.dig_P9 = (((int) paramBuff[23]) << 8) + paramBuff[22];

    calibParam.dig_H1 = (uint8_t) BME280_readByte(BME280_CALIB_DH1_REG, paramBuff);
    
    BME280_readBlock(BME280_CALIB_DH2_LSB_REG, paramBuff, 7);
        
    calibParam.dig_H2 = (((int) paramBuff[1]) << 8) + paramBuff[0];
    calibParam.dig_H3 = (uint8_t) paramBuff[2];
    calibParam.dig_H4 = (((int) paramBuff[3]) << 4) | (paramBuff[4] & 0xF);
    calibParam.dig_H5 = (((int) paramBuff[5]) << 4) | (paramBuff[4] >> 4);
    calibParam.dig_H6 = (short) paramBuff[6];
}

void BME280_readMeasurements(const struct temperature_sensor *const me) {
  
    uint8_t sensorData[BME280_DATA_FRAME_SIZE];

    BME280_readBlock(BME280_PRESS_MSB_REG, sensorData, BME280_DATA_FRAME_SIZE);  

    adc_H = ((uint32_t) sensorData[BME280_HUM_MSB] << 8) |
            sensorData[BME280_HUM_LSB];

    adc_T = ((uint32_t) sensorData[BME280_TEMP_MSB] << 12) |
            (((uint32_t) sensorData[BME280_TEMP_LSB] << 4) |
            ((uint32_t) sensorData[BME280_TEMP_XLSB] >> 4));

    adc_P = ((uint32_t) sensorData[BME280_PRESS_MSB] << 12) |
            (((uint32_t) sensorData[BME280_PRESS_LSB] << 4) |
            ((uint32_t) sensorData[BME280_PRESS_XLSB] >> 4));
}

float Weather_getTemperatureDegC(void) {
    float temperature = (float) BME280_compensateTemperature() / 100;
    return temperature;
}

float Weather_getPressureKPa(void) {
    float pressure = (float) BME280_compensatePressure() / 1000;
    return pressure;
}

float Weather_getHumidityRH(void) {
    float humidity = (float) BME280_compensateHumidity() / 1024;
    return humidity;
}

/* 
 * Returns temperature in DegC, resolution is 0.01 DegC. 
 * Output value of "5123" equals 51.23 DegC.  
 */
static long BME280_compensateTemperature(void) {
    long tempV1, tempV2, t;

    tempV1 = ((((adc_T >> 3) - ((long) calibParam.dig_T1 << 1))) * ((long) calibParam.dig_T2)) >> 11;
    tempV2 = (((((adc_T >> 4) - ((long) calibParam.dig_T1)) * ((adc_T >> 4) - ((long) calibParam.dig_T1))) >> 12)*((long) calibParam.dig_T3)) >> 14;
    t_fine = tempV1 + tempV2;
    t = (t_fine * 5 + 128) >> 8;

    return t;
}

/* 
 * Returns pressure in Pa as unsigned 32 bit integer. 
 * Output value of "96386" equals 96386 Pa = 96.386 kPa 
 */
static uint32_t BME280_compensatePressure(void) {
    long pressV1, pressV2;
    uint32_t p;

    pressV1 = (((long) t_fine) >> 1) - (long) 64000;
    pressV2 = (((pressV1 >> 2) * (pressV1 >> 2)) >> 11) * ((long) calibParam.dig_P6);
    pressV2 = pressV2 + ((pressV1 * ((long) calibParam.dig_P5)) << 1);
    pressV2 = (pressV2 >> 2)+(((long) calibParam.dig_P4) << 16);
    pressV1 = (((calibParam.dig_P3 * (((pressV1 >> 2) * (pressV1 >> 2)) >> 13)) >> 3) +
            ((((long) calibParam.dig_P2) * pressV1) >> 1)) >> 18;
    pressV1 = ((((32768 + pressV1))*((long) calibParam.dig_P1)) >> 15);

    if (pressV1 == 0) {
        // avoid exception caused by division by zero
        return 0;
    }

    p = (((uint32_t) (((long) 1048576) - adc_P)-(pressV2 >> 12)))*3125;
    if (p < 0x80000000) {
        p = (p << 1) / ((uint32_t) pressV1);
    } else {
        p = (p / (uint32_t) pressV1) * 2;
    }

    pressV1 = (((long) calibParam.dig_P9) * ((long) (((p >> 3) * (p >> 3)) >> 13))) >> 12;
    pressV2 = (((long) (p >> 2)) * ((long) calibParam.dig_P8)) >> 13;
    p = (uint32_t) ((long) p + ((pressV1 + pressV2 + calibParam.dig_P7) >> 4));

    return p;
}

/* 
 * Returns humidity in %RH as unsigned 32 bit integer in Q22.10 format
 * (22 integer and 10 fractional bits).
 * Output value of "47445" represents 47445/1024 = 46.333 %RH
 */
static uint32_t BME280_compensateHumidity(void) {
    long humV;
    uint32_t h;

    humV = (t_fine - ((long) 76800));
    humV = (((((adc_H << 14) - (((long) calibParam.dig_H4) << 20) - (((long) calibParam.dig_H5) * humV)) +
            ((long) 16384)) >> 15) * (((((((humV * ((long) calibParam.dig_H6)) >> 10) *
            (((humV * ((long) calibParam.dig_H3)) >> 11) + ((long) 32768))) >> 10) +
            ((long) 2097152)) * ((long) calibParam.dig_H2) + 8192) >> 14));
    humV = (humV - (((((humV >> 15) * (humV >> 15)) >> 7) * ((long) calibParam.dig_H1)) >> 4));
    humV = (humV < 0 ? 0 : humV);
    humV = (humV > 419430400 ? 419430400 : humV);

    h = (uint32_t) (humV >> 12);
    return h;
}