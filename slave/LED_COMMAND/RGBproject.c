//*****************************************************************************
//
// qs-rgb.c - Quickstart for the EK-LM4F120XL Stellaris LaunchPad
//
// Copyright (c) 2012 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 9453 of the EK-LM4F120XL Firmware Package.
//
//*****************************************************************************


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

#include "RGBProject.h"
#include "serial_link.h"
#include "serial_link_frame_protocol.h"

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>EK-LM4F120XL Quickstart Application (qs-rgb)</h1>
//!
//! A demonstration of the Stellaris LaunchPad (EK-LM4F120XL) capabilities.
//!
//! Press and/or hold the left button traverse toward the red end of the
//! ROYGBIV color spectrum.  Press and/or hold the right button to traverse
//! toward the violet end of the ROYGBIV color spectrum.
//!
//! Leave idle for 5 seconds to see a automatically changing color display
//!
//! Press and hold both left and right buttons for 3 seconds to enter
//! hibernation.  During hibernation last color on screen will blink on the
//! LED for 0.5 seconds every 3 seconds.
//!
//! Command line UART protocol can also control the system.
//!
//! Command 'help' to generate list of commands and helpful information.
//! Command 'hib' will place the device into hibernation mode.
//! Command 'rand' will initiate the pseudo-random sequence.
//! Command 'intensity' followed by a number between 0.0 and 1.0 will scale
//! the brightness of the LED by that factor.
//! Command 'rgb' followed by a six character hex value will set the color. For
//! example 'rgb FF0000' will produce a red color.
//
//*****************************************************************************


//*****************************************************************************
//
// Entry counter to track how long to stay in certain staging states before
// making transition into hibernate.
//
//*****************************************************************************
static volatile unsigned long ulHibModeEntryCount;

static char g_cInput[128];

static unsigned long index = 0;

uint32_t m_cpuClock=0;
//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, unsigned long ulLine)
{
}
#endif


void RGBProcess(char* pcStr);
void cbController(void);
void cbSPI(void);
void cbUARTCharReceived(void *pData, uint8_t car);

uint32_t m_nbCharReceived = 0;
SerialLinkConfig_t serial =
{
		.baudrate = B115200,
		.dataSize = BIT_8,
		.parity = PARITY_NONE,
		.cbReception = cbUARTCharReceived,
		.pReceptionData = &m_nbCharReceived,
		.cbEndOfTransmition = NULL
};

//*****************************************************************************
//
// Called by the NVIC as a result of SysTick Timer rollover interrupt flag
//
// Checks buttons and calls AppButtonHandler to manage button events.
// Tracks time and auto mode color stepping.  Calls AppRainbow to implement
// RGB color changes.
//
//*****************************************************************************
void
SysTickIntHandler(void)
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

static uint8_t m_frame[1024];

static uint8_t* AllocData(void)
{
	return m_frame;
//	pData = malloc(1024);
}

static void FreeData(uint8_t *pData)
{
//	free(pData);
	pData = NULL;
}

static uint16_t nbReceivedFrame = 0;
uint16_t m_lastFrameSize = 0;
static void FrameReceived(void* pData, uint8_t *pMsg, uint16_t size)
{
	uint16_t *val = (uint16_t*)pData;

	(*val)++;
//	memcpy(m_frame, pMsg, size);
	m_lastFrameSize = size;
}

static uint8_t comChannel = 0xFF;

//*****************************************************************************
///
/// \brief 		Main function performs init and manages system.
///
/// \details 	Called automatically after the system and compiler pre-init sequences.
/// 			Performs system init calls, restores state from hibernate if needed and
/// 			then manages the application context duties of the system.
///
//*****************************************************************************
static uint8_t car = 0xAA;
int
main(void)
{
    uint32_t ulStatus[3];
    uint32_t ulResetCause;
    int32_t lCommandStatus;
    uint32_t inpuData;
    //
    // Enable stacking for interrupt handlers.  This allows floating-point
    // instructions to be used within interrupt handlers, but at the expense of
    // extra stack usage.
    //
    ROM_FPUEnable();
    ROM_FPUStackingEnable();






    //
    // Set the system clock to run at 40Mhz off PLL with external crystal as
    // reference.
    //
    ROM_SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);


    //
    // Enable and Initialize the UART.
    //
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    m_cpuClock = SysCtlClockGet();

    //------------ SPI Test -----------------
//
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
//    GPIOPinConfigure(GPIO_PB5_SSI2FSS);
//    GPIOPinConfigure(GPIO_PB4_SSI2CLK);
//    GPIOPinConfigure(GPIO_PB7_SSI2TX);
//    GPIOPinConfigure(GPIO_PB6_SSI2RX);
//    GPIOPinTypeSSI(GPIO_PORTB_BASE, GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7);
//
//    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI2);
//    SSIConfigSetExpClk(SSI2_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_SLAVE, 10000, 16);
//
//    SSIIntRegister(SSI2_BASE, cbSPI);
//    SSIIntEnable(SSI2_BASE, NULL);
    //------------ End SPI Test -----------------

//------------------ UART Com Test ---------------------
   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
   GPIOPinConfigure(GPIO_PB0_U1RX);
   GPIOPinConfigure(GPIO_PB1_U1TX);
   GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

   SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);
   UARTConfigSetExpClk(UART1_BASE, m_cpuClock, 9600,
                           (UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE |
                            UART_CONFIG_WLEN_8));

   //----------------- End UART Com Test------------------------

//    UARTStdioInit(0);
//
//    UARTprintf("Welcome to the Stellaris LM4F120 LaunchPad!\n");
//    UARTprintf("Type 'help' for a list of commands\n");
//    UARTprintf("> ");

//    SerialLink_Init(0, &serial);
//    SerialLink_Write(0, "Hello World !!!", 16);
   comChannel = SerialLinkFrameProtocoleInit(SERIAL1,
		   B115200,
		   BIT_8,
		   PARITY_NONE,
		   STOP_BIT_1,
		   (cbNotifyRx_t)FrameReceived,
		   &nbReceivedFrame,
		   (cbAllocMsg_t)AllocData,
		   (cbFreeMsg_t)FreeData);
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
    IntMasterEnable();



    //
    // spin forever and wait for carriage returns or state changes.
    //
    while(1)
    {

//   	UARTprintf("%d\n", ulStatus );
//    	ulStatus = SysTickValueGet();
//    	SysCtlDelay(SysCtlClockGet()/(10000*3));
//    	RGBColorSet(ulStatus);
//    	ulStatus[0] = (ulStatus[0]+1)%65536;
//    	ulStatus[1] = (ulStatus[1]+3)%65536;
//    	ulStatus[2] = (ulStatus[2]+6)%65536;

        //
        // Peek to see if a full command is ready for processing
        //
//        while(UARTPeek('\r') == -1)
//        {
//            //
//            // millisecond delay.  A SysCtlSleep() here would also be OK.
//            //
//            SysCtlDelay(SysCtlClockGet() / (1000 / 3));
//
//        }
//        UARTgets(g_cInput,sizeof(g_cInput));
//        RGBProcess(g_cInput);
//    	SysCtlDelay(2 * (SysCtlClockGet() / 3));
//    	UARTCharPut(UART1_BASE, car);
//    	UARTCharPut(UART0_BASE, 'A');
//    	SerialLink_Write(0, "Hello World !!!\n", 16);
    }
}

void RGBProcess(char* pcStr)
{
	// Format "XXXXXX X\n" -> RRGGBB I
    uint32_t ulHTMLColor;
    uint32_t ulColors[3];
    T_COLOR couleur;

        couleur.value = ustrtoul(pcStr, 0, 16);

        ulColors[RED] 	= (uint32_t)(couleur.color.red << 8);
        ulColors[GREEN] = (uint32_t)(couleur.color.green << 8);
        ulColors[BLUE] 	= (uint32_t)(couleur.color.blue << 8);

//        ulColors[RED] 	= 0xFFFF;
//        ulColors[GREEN] = 0xFFFF;
//        ulColors[BLUE] 	= 0xFFFF;



        RGBColorSet(ulColors);


}

static uint32_t status;
static uint8_t state;

void cbController(void)
{

	status = GPIOPinIntStatus(GPIO_PORTB_BASE, false);
	state = GPIOPinRead(GPIO_PORTB_BASE, GPIO_PIN_5);
	GPIOPinIntClear(GPIO_PORTB_BASE, GPIO_PIN_5);
}

void cbSPI(void)
{

}

//FIXME for test. Must not be here
static uint8_t testReceivedChar;
void cbUARTCharReceived(void *pData, uint8_t car)
{
	uint32_t *number = (uint32_t*)pData;
	testReceivedChar = car;
	(*number)++;
}
