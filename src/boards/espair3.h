#define NAME "ESP Air3"
#define PIN_RX_TO_S8 14
#define PIN_TX_TO_S8 12
#define BME_ENABLED
#define BME_I2C_ADDRESS 0x77

const char *boardInfo()
{
    return "🎈";
}