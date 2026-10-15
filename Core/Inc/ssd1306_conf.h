/**
 * Configuration file for SSD1306 OLED library.
 * Configured for STM32F103, I2C1 (PB6=SCL, PB7=SDA).
 */

#ifndef __SSD1306_CONF_H__
#define __SSD1306_CONF_H__

// Microcontroller family
#ifndef STM32F1
#define STM32F1
#endif

// Interface selection
#define SSD1306_USE_I2C

// I2C Configuration
#define SSD1306_I2C_PORT        hi2c1
#define SSD1306_I2C_ADDR        (0x3C << 1)

// Screen dimensions (default 128x64)
#define SSD1306_WIDTH           128
#define SSD1306_HEIGHT          64

// Mirror screen if needed
// #define SSD1306_MIRROR_VERT
// #define SSD1306_MIRROR_HORIZ

// Invert color if needed
// #define SSD1306_INVERSE_COLOR

// Include fonts
#define SSD1306_INCLUDE_FONT_6x8
#define SSD1306_INCLUDE_FONT_7x10
#define SSD1306_INCLUDE_FONT_11x18
#define SSD1306_INCLUDE_FONT_16x26

// Function aliases for compatibility
#define ssd1306_DrawLine ssd1306_Line

#endif /* __SSD1306_CONF_H__ */
