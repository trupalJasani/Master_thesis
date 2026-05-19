#include "leaf_wetness.h"
#include <stddef.h>

#define DAVIS_OK      0
#define DAVIS_ERROR  -1

/* Expose the API to main.c */
Davis6420_Drv_t DAVIS6420_Driver = {
    Davis6420_Init,
    Davis6420_DeInit,
    Davis6420_GetWetnessRaw,
    Davis6420_IsLeafWet
};

int32_t Davis6420_RegisterBusIO(Davis6420_Object_t *pObj, Davis6420_IO_t *pIO) {
    if (pObj == NULL || pIO == NULL) return DAVIS_ERROR;

    pObj->IO.Init    = pIO->Init;
    pObj->IO.DeInit  = pIO->DeInit;
    pObj->IO.ReadADC = pIO->ReadADC;

    if (pObj->IO.Init != NULL) {
        if (pObj->IO.Init() != DAVIS_OK) return DAVIS_ERROR;
    }
    return DAVIS_OK;
}

int32_t Davis6420_Init(Davis6420_Object_t *pObj) {
    if (pObj == NULL) return DAVIS_ERROR;

    /* Set calibration threshold. 
       Voltage drops as water bridges the grid. Tune this during lab testing! */
    pObj->wet_threshold = 2000; 
    pObj->is_initialized = 1;
    
    return DAVIS_OK;
}

int32_t Davis6420_DeInit(Davis6420_Object_t *pObj) {
    if (pObj == NULL) return DAVIS_ERROR;
    if (pObj->IO.DeInit != NULL) pObj->IO.DeInit();
    pObj->is_initialized = 0;
    return DAVIS_OK;
}

int32_t Davis6420_GetWetnessRaw(Davis6420_Object_t *pObj, uint32_t *Value) {
    if (pObj == NULL || pObj->is_initialized == 0 || pObj->IO.ReadADC == NULL) {
        return DAVIS_ERROR;
    }
    return pObj->IO.ReadADC(Value);
}

int32_t Davis6420_IsLeafWet(Davis6420_Object_t *pObj, bool *IsWet) {
    uint32_t raw_val = 0;
    if (Davis6420_GetWetnessRaw(pObj, &raw_val) != DAVIS_OK) {
        return DAVIS_ERROR;
    }
    
    /* Agronomic Logic: Compare current reading against calibration threshold */
    *IsWet = (raw_val >= pObj->wet_threshold) ? true : false;
    
    return DAVIS_OK;
}