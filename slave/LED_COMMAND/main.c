/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup main
/// \author Dev
///
///	\brief
///
///
/// \file
/// \brief Fichier source du module main
///
///
///
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <main.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_hibernate.h"
#include "inc/hw_ints.h"

#include "driverlib/fpu.h"
#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/ssi.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/hibernate.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "utils/cmdline.h"
#include "utils/ustdlib.h"
#include "drivers/rgb.h"
#include "drivers/buttons.h"

#include "control.h"

/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////

uint32_t m_cpuClock=0;

/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
void RGBProcess(char* pcStr);
void cbController(void);
void cbUARTCharReceived(void *pData, uint8_t car);
bool cbUARTCharToTransmit(void* pData, uint8_t *car);

/////////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
void SysTickIntHandler(void)
{
	unsigned long ulColors[3];
	static unsigned char index2=0;

//	if(index==0)
//	{
//		if ((index2 % 2) == 0)
//		{
//			ulColors[RED] = 0;
//			ulColors[GREEN] = 0;
//			ulColors[BLUE] = 0;
//		}
//		else
//		{
//			ulColors[RED] = 0xFFFF;
//			ulColors[GREEN] = 0xFFFF;
//			ulColors[BLUE] = 0xFFFF;
//		}
//
//		index2++;

//		RGBColorSet(ulColors);
//	}
//    index = (index + 1) % 4;



}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
void main(void)
{
    //
    // Enable stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPUEnable();
    ROM_FPUStackingEnable();



    //
    // Set the system clock to run at 80Mhz off PLL with external crystal as
    // reference.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);


    //
    // Enable and Initialize the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    m_cpuClock = SysCtlClockGet();

    Control_init();

    //
    // Initialize the RGB LED. AppRainbow typically only called from interrupt
    // context. Safe to call here to force initial color update because
    // interrupts are not yet enabled.
    //
    RGBInit(0);
    RGBIntensitySet(0.5);

    RGBEnable();

    //
    // Initialize the buttons
    //
    ButtonsInit();

    //
    // Initialize the SysTick interrupt to process colors and buttons.
    //
//    SysTickPeriodSet(SysCtlClockGet());
//    SysTickEnable();
//    SysTickIntEnable();
//    SysTickIntRegister(SysTickIntHandler);
    IntMasterEnable();

    //
    // spin forever and wait for carriage returns or state changes.
    //
    while(1)
    {

    }
}

#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
void RGBProcess(char* pcStr)
{
	// Format "XXXXXX X\n" -> RRGGBB I
    uint32_t ulColors[3];
    T_COLOR couleur;

        couleur.value = ustrtoul(pcStr, 0, 16);

        ulColors[RED] 	= (uint32_t)(couleur.color.red << 8);
        ulColors[GREEN] = (uint32_t)(couleur.color.green << 8);
        ulColors[BLUE] 	= (uint32_t)(couleur.color.blue << 8);

//        ulColors[RED] 	= 0xFFFF;
//        ulColors[GREEN] = 0xFFFF;
//        ulColors[BLUE] 	= 0xFFFF;



        RGBColorSet((volatile unsigned long*)ulColors);


}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
void RGB_SetColor(tRGBControl* color)
{
	uint32_t ulColors[3];

    ulColors[RED] 	= (uint32_t)(color->red);
    ulColors[GREEN] = (uint32_t)(color->green);
    ulColors[BLUE] 	= (uint32_t)(color->blue);

    RGBColorSet(ulColors);

}

/////////////////////////////////////////////////////////////////////////////////
// Private functions
/////////////////////////////////////////////////////////////////////////////////


///
/// \}
///
