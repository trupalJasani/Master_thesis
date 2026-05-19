/**
  ******************************************************************************
  * @file    leaf_wetness.h
  * @author  Trupal Jasani
  * @brief   leaf_wetness header driver file
  ******************************************************************************
  */

#ifndef LEAF_WETNESS_H
#define LEAF_WETNESS_H

#include <stdint.h>
#include <stdbool.h>

/* --- 1. Bus IO Structure (Hardware Abstraction) --- */
typedef struct {
    int32_t (*Init)(void);
    int32_t (*DeInit)(void);
    int32_t (*ReadADC)(uint32_t *Value); 
} Davis6420_IO_t;

/* --- 2. Sensor Object Structure --- */
typedef struct {
    Davis6420_IO_t IO;
    uint8_t        is_initialized;
    uint32_t       wet_threshold; // Agronomic threshold for "wet" vs "dry"
} Davis6420_Object_t;

/* --- 3. Component Driver API --- */
typedef struct {
    int32_t (*Init)(Davis6420_Object_t *pObj);
    int32_t (*DeInit)(Davis6420_Object_t *pObj);
    int32_t (*GetWetnessRaw)(Davis6420_Object_t *pObj, uint32_t *Value);
    int32_t (*IsLeafWet)(Davis6420_Object_t *pObj, bool *IsWet);
} Davis6420_Drv_t;

/* --- Exported Variables & Functions --- */
extern Davis6420_Drv_t DAVIS6420_Driver;

int32_t Davis6420_RegisterBusIO(Davis6420_Object_t *pObj, Davis6420_IO_t *pIO);
int32_t Davis6420_Init(Davis6420_Object_t *pObj);
int32_t Davis6420_DeInit(Davis6420_Object_t *pObj);
int32_t Davis6420_GetWetnessRaw(Davis6420_Object_t *pObj, uint32_t *Value);
int32_t Davis6420_IsLeafWet(Davis6420_Object_t *pObj, bool *IsWet);

#endif /* DAVIS_6420_H */