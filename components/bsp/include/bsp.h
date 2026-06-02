#ifndef BSP_H
#define BSP_H

#include <stdint.h>
#include <stdbool.h>

/* ========================================================================= */
/* HARDWARE PIN MAPPING (ESP32-C3)                                           */
/* ========================================================================= */
/* Power Management */
#define BSP_MOSFET_PIN          10 // Gates power to all external sensors

/* I2C (Sensirion SHT31) */
#define BSP_I2C_PORT            0
#define BSP_I2C_SDA_PIN         8
#define BSP_I2C_SCL_PIN         9

/* ADC (Davis 6420 Leaf Wetness & Soil Moisture) */
/* Note: ESP32-C3 ADC1 uses GPIO 0, 1, 2, 3, 4 */
#define BSP_ADC_DAVIS_CHAN      2  // Mapped to ADC1_CHANNEL_2 (GPIO 2)
#define BSP_ADC_SOIL_CHAN       3  // Mapped to ADC1_CHANNEL_3 (GPIO 3)

/* UART (Seeed Wio-E5 LoRa) */
#define BSP_UART_PORT           1
#define BSP_UART_TX_PIN         4
#define BSP_UART_RX_PIN         5
#define BSP_UART_BAUD_RATE      9600

/* ========================================================================= */
/* SYSTEM FUNCTIONS                                                          */
/* ========================================================================= */
void bsp_init_all(void);
void bsp_power_sensors(bool enable);
void bsp_delay_ms(uint32_t ms);

/* ========================================================================= */
/* ADAPTER WRAPPERS (Injected into Object Drivers)                           */
/* ========================================================================= */

/* SHT31 Wrappers */
int32_t bsp_i2c_write(uint16_t addr, uint8_t *p_data, uint16_t len);
int32_t bsp_i2c_read(uint16_t addr, uint8_t *p_data, uint16_t len);

/* ADC Wrappers */
int32_t bsp_adc_read_davis(uint32_t *raw_value);
int32_t bsp_adc_read_soil(uint32_t *raw_value);

/* LoRa UART Wrappers */
int32_t bsp_uart_write(const uint8_t *p_data, uint16_t len);
int32_t bsp_uart_read(uint8_t *p_data, uint16_t len);

#endif /* BSP_H */