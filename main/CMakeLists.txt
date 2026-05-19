idf_component_register(SRCS "main.c" "logic_smith_period.c"
                    INCLUDE_DIRS "."
                    REQUIRES bsp sht31 leaf_wetness_sensor soil_moisture wio_e5)