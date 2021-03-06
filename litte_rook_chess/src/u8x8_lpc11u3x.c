/*
  u8x8_lpc11u3x.c
*/

#include "chip.h"
#include "delay.h"
#include "u8x8.h"

#define I2C_CLOCK_PIN 6
#define I2C_CLOCK_PORT 0
#define I2C_DATA_PIN  21
#define I2C_DATA_PORT 0

#define KEY_SELECT_PIN 14
#define KEY_SELECT_PORT 0
#define KEY_SELECT_FN (IOCON_FUNC1|IOCON_DIGMODE_EN)

#define KEY_PREV_PIN 19
#define KEY_PREV_PORT 1

#define KEY_NEXT_PIN 1
#define KEY_NEXT_PORT 0

#define KEY_HOME_PIN 15
#define KEY_HOME_PORT 0
#define KEY_HOME_FN (IOCON_FUNC1|IOCON_DIGMODE_EN)

uint8_t u8x8_gpio_and_delay_lpc11u3x(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:
      /* only support for software I2C*/
      Chip_IOCON_PinMuxSet(LPC_IOCON, I2C_CLOCK_PORT, I2C_CLOCK_PIN, IOCON_MODE_PULLUP);
      Chip_IOCON_PinMuxSet(LPC_IOCON, I2C_DATA_PORT, I2C_DATA_PIN, IOCON_MODE_PULLUP);
      Chip_GPIO_SetPinDIRInput(LPC_GPIO, I2C_CLOCK_PORT, I2C_CLOCK_PIN);
      Chip_GPIO_SetPinDIRInput(LPC_GPIO, I2C_DATA_PORT, I2C_DATA_PIN);    

      Chip_IOCON_PinMuxSet(LPC_IOCON, KEY_SELECT_PORT, KEY_SELECT_PIN, KEY_SELECT_FN|IOCON_MODE_PULLUP);    
      Chip_GPIO_SetPinDIRInput(LPC_GPIO, KEY_SELECT_PORT, KEY_SELECT_PIN);

      Chip_IOCON_PinMuxSet(LPC_IOCON, KEY_PREV_PORT, KEY_PREV_PIN, IOCON_MODE_PULLUP);    
      Chip_GPIO_SetPinDIRInput(LPC_GPIO, KEY_PREV_PORT, KEY_PREV_PIN);

      Chip_IOCON_PinMuxSet(LPC_IOCON, KEY_NEXT_PORT, KEY_NEXT_PIN, IOCON_MODE_PULLUP);    
      Chip_GPIO_SetPinDIRInput(LPC_GPIO, KEY_NEXT_PORT, KEY_NEXT_PIN);

      Chip_IOCON_PinMuxSet(LPC_IOCON, KEY_HOME_PORT, KEY_HOME_PIN, KEY_HOME_FN|IOCON_MODE_PULLUP);    
      Chip_GPIO_SetPinDIRInput(LPC_GPIO, KEY_HOME_PORT, KEY_HOME_PIN);

      break;
    case U8X8_MSG_DELAY_NANO:
      /* not required for SW I2C */
      break;
    
    case U8X8_MSG_DELAY_10MICRO:
      /* not used at the moment */
      break;
    
    case U8X8_MSG_DELAY_100NANO:
      /* not used at the moment */
      break;
   
    case U8X8_MSG_DELAY_MILLI:
      delay_micro_seconds(arg_int*1000UL);
      break;
    case U8X8_MSG_DELAY_I2C:
      /* arg_int is 1 or 4: 100KHz (5us) or 400KHz (1.25us) */
      delay_micro_seconds(arg_int<=2?5:1);
      break;
    
    case U8X8_MSG_GPIO_I2C_CLOCK:
      if ( arg_int == 0 )
      {
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, I2C_CLOCK_PORT, I2C_CLOCK_PIN);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, I2C_CLOCK_PORT, I2C_CLOCK_PIN);
      }
      else
      {
	//Chip_GPIO_SetPinOutHigh(LPC_GPIO, I2C_CLOCK_PORT, I2C_CLOCK_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, I2C_CLOCK_PORT, I2C_CLOCK_PIN);
      }
      break;
    case U8X8_MSG_GPIO_I2C_DATA:
      if ( arg_int == 0 )
      {
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, I2C_DATA_PORT, I2C_DATA_PIN);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, I2C_DATA_PORT, I2C_DATA_PIN);
      }
      else
      {
	//Chip_GPIO_SetPinOutHigh(LPC_GPIO, I2C_DATA_PORT, I2C_DATA_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, I2C_DATA_PORT, I2C_DATA_PIN);
      }
      break;
    case U8X8_MSG_GPIO_MENU_SELECT:
      u8x8_SetGPIOResult(u8x8, Chip_GPIO_GetPinState(LPC_GPIO, KEY_SELECT_PORT, KEY_SELECT_PIN));
      break;
    case U8X8_MSG_GPIO_MENU_NEXT:
      u8x8_SetGPIOResult(u8x8, Chip_GPIO_GetPinState(LPC_GPIO, KEY_NEXT_PORT, KEY_NEXT_PIN));
      break;
    case U8X8_MSG_GPIO_MENU_PREV:
      u8x8_SetGPIOResult(u8x8, Chip_GPIO_GetPinState(LPC_GPIO, KEY_PREV_PORT, KEY_PREV_PIN));
      break;
    
    case U8X8_MSG_GPIO_MENU_HOME:
      u8x8_SetGPIOResult(u8x8, Chip_GPIO_GetPinState(LPC_GPIO, KEY_HOME_PORT, KEY_HOME_PIN));
      break;
    default:
      u8x8_SetGPIOResult(u8x8, 1);
      break;
  }
  return 1;
}
