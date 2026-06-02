#include "bsp.h"

/* ESP-IDF Specific Headers */
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "esp_adc/adc_oneshot.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* Global handle for the ADC unit */
static adc_oneshot_unit_handle_t adc1_handle;

/* ========================================================================= */
/* INITIALIZATION & POWER                                                    */
/* ========================================================================= */
void bsp_init_all(void) {
    /* 1. Initialize MOSFET Power Pin (GPIO 10) */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BSP_MOSFET_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);
    bsp_power_sensors(false); /* Start with sensors powered off */

    /* 2. Initialize I2C (Sensirion SHT31) */
    i2c_config_t i2c_conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = BSP_I2C_SDA_PIN,
        .scl_io_num = BSP_I2C_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };
    i2c_param_config(BSP_I2C_PORT, &i2c_conf);
    i2c_driver_install(BSP_I2C_PORT, i2c_conf.mode, 0, 0, 0);

    /* 3. Initialize ADC (Davis 6420 Leaf Wetness & Soil Moisture) */
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    adc_oneshot_new_unit(&init_config1, &adc1_handle);

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_12, /* DB_12 allows reading up to ~3.3V on ESP32-C3 */
    };
    adc_oneshot_config_channel(adc1_handle, BSP_ADC_DAVIS_CHAN, &config);
    adc_oneshot_config_channel(adc1_handle, BSP_ADC_SOIL_CHAN, &config);

    /* 4. Initialize UART (Seeed Wio-E5 LoRa Radio) */
    uart_config_t uart_config = {
        .baud_rate = BSP_UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(BSP_UART_PORT, 256, 0, 0, NULL, 0);
    uart_param_config(BSP_UART_PORT, &uart_config);
    uart_set_pin(BSP_UART_PORT, BSP_UART_TX_PIN, BSP_UART_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}

void bsp_power_sensors(bool enable) {
    /* Assuming an N-Channel MOSFET: High (1) = ON, Low (0) = OFF */
    gpio_set_level(BSP_MOSFET_PIN, enable ? 1 : 0);
    
    /* Give analog and digital sensors time to stabilize after power is applied */
    if (enable) {
        bsp_delay_ms(50);
    }
}

void bsp_delay_ms(uint32_t ms) {
    vTaskDelay(pdMS_TO_TICKS(ms));
}

/* ========================================================================= */
/* I2C WRAPPERS (SHT31)                                                      */
/* ========================================================================= */
int32_t bsp_i2c_write(uint16_t addr, uint8_t *p_data, uint16_t len) {
    esp_err_t err = i2c_master_write_to_device(BSP_I2C_PORT, addr, p_data, len, pdMS_TO_TICKS(1000));
    return (err == ESP_OK) ? 0 : -1;
}

int32_t bsp_i2c_read(uint16_t addr, uint8_t *p_data, uint16_t len) {
    esp_err_t err = i2c_master_read_from_device(BSP_I2C_PORT, addr, p_data, len, pdMS_TO_TICKS(1000));
    return (err == ESP_OK) ? 0 : -1;
}

/* ========================================================================= */
/* ADC WRAPPERS (Davis Leaf Wetness & Soil Moisture)                         */
/* ========================================================================= */
int32_t bsp_adc_read_davis(uint32_t *raw_value) {
    int out_raw;
    esp_err_t err = adc_oneshot_read(adc1_handle, BSP_ADC_DAVIS_CHAN, &out_raw);
    if (err == ESP_OK) {
        *raw_value = (uint32_t)out_raw;
        return 0;
    }
    return -1;
}

int32_t bsp_adc_read_soil(uint32_t *raw_value) {
    int out_raw;
    esp_err_t err = adc_oneshot_read(adc1_handle, BSP_ADC_SOIL_CHAN, &out_raw);
    if (err == ESP_OK) {
        *raw_value = (uint32_t)out_raw;
        return 0;
    }
    return -1;
}

/* ========================================================================= */
/* UART WRAPPERS (Wio-E5 LoRa Radio)                                         */
/* ========================================================================= */
int32_t bsp_uart_write(const uint8_t *p_data, uint16_t len) {
    int written = uart_write_bytes(BSP_UART_PORT, (const char *)p_data, len);
    return (written == len) ? 0 : -1;
}

int32_t bsp_uart_read(uint8_t *p_data, uint16_t len) {
    /* 500ms timeout provides ample time to catch the AT response */
    int rxBytes = uart_read_bytes(BSP_UART_PORT, p_data, len, pdMS_TO_TICKS(500));
    return (rxBytes >= 0) ? 0 : -1;
}