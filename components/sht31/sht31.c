#include "sht31.h"
#include <stddef.h>

#define SHT31_OK      0
#define SHT31_ERROR  -1

/* The default I2C Address for the Sensirion SHT31 */
#define SHT31_DEFAULT_I2C_ADDR 0x44 

/* Define the generic driver structure for main.c to use */
SHT31_Drv_t SHT31_Driver = {
    SHT31_Init,
    SHT31_DeInit,
    SHT31_GetTempHum
};

/**
  * @brief  Register Component Bus IO operations
  * @param  pObj the device object
  * @param  pIO the hardware IO pointers provided by the BSP
  * @retval 0 in case of success, an error code otherwise
  */
int32_t SHT31_RegisterBusIO(SHT31_Object_t *pObj, SHT31_IO_t *pIO) {
    if (pObj == NULL || pIO == NULL) {
        return SHT31_ERROR;
    }

    /* Map the hardware functions to the object */
    pObj->IO.Init   = pIO->Init;
    pObj->IO.DeInit = pIO->DeInit;
    pObj->IO.Write  = pIO->Write;
    pObj->IO.Read   = pIO->Read;
    pObj->IO.Delay  = pIO->Delay;

    /* Initialize the physical I2C bus if an Init function was provided */
    if (pObj->IO.Init != NULL) {
        if (pObj->IO.Init() != SHT31_OK) {
            return SHT31_ERROR;
        }
    }

    return SHT31_OK;
}

/**
  * @brief  Initialize the SHT31 sensor logic
  */
int32_t SHT31_Init(SHT31_Object_t *pObj) {
    if (pObj == NULL) {
        return SHT31_ERROR;
    }

    pObj->i2c_address = SHT31_DEFAULT_I2C_ADDR;
    pObj->is_initialized = 1;
    
    return SHT31_OK;
}

/**
  * @brief  Deinitialize the sensor object
  */
int32_t SHT31_DeInit(SHT31_Object_t *pObj) {
    if (pObj == NULL) {
        return SHT31_ERROR;
    }
    
    if (pObj->IO.DeInit != NULL) {
        pObj->IO.DeInit();
    }

    pObj->is_initialized = 0;
    return SHT31_OK;
}

/**
  * @brief  Trigger a measurement and read the Temperature and Humidity
  */
int32_t SHT31_GetTempHum(SHT31_Object_t *pObj, float *Temperature, float *Humidity) {
    /* 1. Ensure object is valid and IO pointers exist */
    if (pObj == NULL || pObj->is_initialized == 0 || 
        pObj->IO.Write == NULL || pObj->IO.Read == NULL || pObj->IO.Delay == NULL) {
        return SHT31_ERROR;
    }

    uint8_t data_rx[6];
    
    /* Command: High Repeatability, Clock Stretching Disabled */
    uint8_t measure_cmd[2] = {0x24, 0x00}; 

    /* 2. Write the command to the sensor */
    if (pObj->IO.Write(pObj->i2c_address, measure_cmd, 2) != SHT31_OK) {
        return SHT31_ERROR;
    }

    /* 3. Wait for the sensor to physically measure the air */
    /* Sensirion datasheet specifies ~15ms for high repeatability. We wait 20ms to be safe. */
    pObj->IO.Delay(20);

    /* 4. Read the 6 bytes of data back */
    if (pObj->IO.Read(pObj->i2c_address, data_rx, 6) != SHT31_OK) {
        return SHT31_ERROR;
    }

    /* 5. Parse Data & Apply Datasheet Math */
    /* Byte 0: Temp MSB, Byte 1: Temp LSB, Byte 2: Temp CRC */
    uint16_t raw_temp = (data_rx[0] << 8) | data_rx[1];
    
    /* Byte 3: Hum MSB, Byte 4: Hum LSB, Byte 5: Hum CRC */
    uint16_t raw_hum = (data_rx[3] << 8) | data_rx[4];

    *Temperature = -45.0f + (175.0f * ((float)raw_temp / 65535.0f));
    *Humidity = 100.0f * ((float)raw_hum / 65535.0f);

    return SHT31_OK;
}