#define NAME "ESP Air1"
#define PIN_RX_TO_S8 D7
#define PIN_TX_TO_S8 D8
#define BME_ENABLED
#define BME_I2C_ADDRESS 0x76

const char *boardInfo()
{
    return "Hello from ESP Air1 🎉";
}