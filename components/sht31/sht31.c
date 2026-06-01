#include "sht31.h"
#include <stddef.h>

#define SHT31_OK      0
#define SHT31_ERROR  -1
#define SHT31_ERROR_CRC  -2
#define SHT31_DEFAULT_I2C_ADDR 0x44 
#define SHT31_CRC_POLYNOMIAL 0x31

static uint8_t SHT31_CalculateCRC(const uint8_t *data, uint8_t length);


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

static uint8_t SHT31_CalculateCRC(const uint8_t *data, uint8_t length) {
    uint8_t crc = 0xFF; // Sensirion defines 0xFF as the initial value

    for (uint8_t i = 0; i < length; i++) {
        crc ^= data[i]; // XOR the current byte into the CRC
        
        /* Process each of the 8 bits */
        for (uint8_t bit = 8; bit > 0; --bit) {
            if (crc & 0x80) {
                /* If the highest bit is 1, shift and XOR with the polynomial */
                crc = (crc << 1) ^ SHT31_CRC_POLYNOMIAL;
            } else {
                /* Otherwise, just shift */
                crc = (crc << 1);
            }
        }
    }
    return crc;
}

int32_t SHT31_GetTempHum(SHT31_Object_t *pObj, float *pTemp, float *pHum) {
    if (pObj == NULL || pObj->is_initialized == 0) return SHT31_ERROR;

    /* 1. Send High Repeatability Measurement Command (0x2400) */
    uint8_t cmd[2] = {0x24, 0x00};
    if (pObj->IO.Write(0x44, cmd, 2) != SHT31_OK) return SHT31_ERROR;

    /* 2. Wait for measurement to complete (15ms max per datasheet) */
    pObj->IO.Delay(15);

    /* 3. Read the 6 bytes of response data */
    uint8_t rx_buf[6] = {0};
    if (pObj->IO.Read(0x44, rx_buf, 6) != SHT31_OK) return SHT31_ERROR;

    /* ========================================================== */
    /* 4. PERFORM CYCLIC REDUNDANCY CHECKS (CRC)                  */
    /* ========================================================== */
    
    /* Check Temperature (Bytes 0 and 1 against Byte 2) */
    if (SHT31_CalculateCRC(&rx_buf[0], 2) != rx_buf[2]) {
        return SHT31_ERROR_CRC; // Temperature data corrupted!
    }

    /* Check Humidity (Bytes 3 and 4 against Byte 5) */
    if (SHT31_CalculateCRC(&rx_buf[3], 2) != rx_buf[5]) {
        return SHT31_ERROR_CRC; // Humidity data corrupted!
    }

    /* ========================================================== */
    /* 5. MATH IS SAFE: Convert to Floating Point                 */
    /* ========================================================== */
    uint16_t raw_temp = (rx_buf[0] << 8) | rx_buf[1];
    uint16_t raw_hum  = (rx_buf[3] << 8) | rx_buf[4];

    /* Formulas strictly from the Sensirion datasheet */
    if (pTemp != NULL) {
        *pTemp = -45.0f + (175.0f * ((float)raw_temp / 65535.0f));
    }
    if (pHum != NULL) {
        *pHum = 100.0f * ((float)raw_hum / 65535.0f);
    }

    return SHT31_OK;
}