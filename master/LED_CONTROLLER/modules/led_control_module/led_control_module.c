/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup led_control_module
/// \{
/// \author Dev
///
///	\brief 
///
///
/// \file
/// \brief Fichier source du module led_control_module
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

#include <ti/sysbios/hal/Timer.h>

#include "inc/hw_ints.h"
#include "utils/softssi.h"
#include "utils/softuart.h"

#include "system.h"

#include "common.h"
#include "led_control_module.h"
/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define BUFFER_SIZE	16
/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
tSoftSSI m_ssiHandle;
tSoftUART m_uartHandle;
uint16_t m_rxBuffer[BUFFER_SIZE];
uint8_t m_txBuffer[BUFFER_SIZE];

Timer_Handle m_timerTxHandle;
Timer_Params m_timerTxParams;
Error_Block m_timerTxEb;

Timer_Handle m_timerRxHandle;
Timer_Params m_timerRxParams;
Error_Block m_timerRxEb;

static Hwi_Struct hwiStructGpioRx;


uint64_t m_intNumber = 0;
/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
static void SSICallback(void);
static void UARTCallback(void);
static void UARTGpioCallback(uint32_t arg);
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
bool LED_CONTROL_Init(void)
{
#if 0
	SoftSSIClkGPIOSet(&m_ssiHandle, GPIO_PORTD_BASE, GPIO_PIN_3);
	SoftSSITxGPIOSet(&m_ssiHandle, GPIO_PORTD_BASE, GPIO_PIN_0);
	SoftSSIRxGPIOSet(&m_ssiHandle, GPIO_PORTD_BASE, GPIO_PIN_1);
	SoftSSIFssGPIOSet(&m_ssiHandle, GPIO_PORTE_BASE, GPIO_PIN_4);

	//
	// Set the callback function used for this software SSI.
	//
	SoftSSICallbackSet(&m_ssiHandle, SSICallback);
	SoftSSITxBufferSet(&m_ssiHandle, m_txBuffer, BUFFER_SIZE);
	SoftSSIRxBufferSet(&m_ssiHandle, m_rxBuffer, BUFFER_SIZE);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	SoftSSIConfigSet(&m_ssiHandle, SOFTSSI_FRF_MOTO_MODE_0, 16);

	SoftSSIEnable(&m_ssiHandle);
#endif

	SoftUARTTxGPIOSet(&m_uartHandle, GPIO_PORTC_BASE, GPIO_PIN_6);
	SoftUARTRxGPIOSet(&m_uartHandle, GPIO_PORTE_BASE, GPIO_PIN_5);

	//
	// Set the callback function used for this software SSI.
	//
	SoftUARTCallbackSet(&m_uartHandle, UARTCallback);
	SoftUARTTxBufferSet(&m_uartHandle, m_txBuffer, BUFFER_SIZE);
	SoftUARTRxBufferSet(&m_uartHandle, m_rxBuffer, BUFFER_SIZE);

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);

	SoftUARTConfigSet(&m_uartHandle, (SOFTUART_CONFIG_WLEN_8 | SOFTUART_CONFIG_STOP_ONE | SOFTUART_CONFIG_PAR_NONE));

	SoftUARTEnable(&m_uartHandle);

	//TODO add hwi for rx interrupt
    Hwi_construct(&hwiStructGpioRx, INT_GPIOE_TM4C129, UARTGpioCallback, NULL, NULL);

	// Init Timer at 20Khz wich call SoftUARTTimerTick
	Error_init(&m_timerTxEb);
	Timer_Params_init(&m_timerTxParams);
	m_timerTxParams.period = 104;
	m_timerTxParams.periodType = Timer_PeriodType_MICROSECS;
	m_timerTxParams.runMode = Timer_RunMode_CONTINUOUS;
	m_timerTxParams.startMode = Timer_StartMode_USER;
	m_timerTxParams.arg = (xdc_UArg)&m_uartHandle;
	m_timerTxHandle = Timer_create(Timer_ANY, (Timer_FuncPtr)SoftUARTTxTimerTick, &m_timerTxParams, &m_timerTxEb);
	if(m_timerTxHandle == NULL)
	{
		return false;
	}

	Timer_start(m_timerTxHandle);

	// Init Timer at 20Khz wich call SoftUARTTimerTick
	Error_init(&m_timerRxEb);
	Timer_Params_init(&m_timerRxParams);
	m_timerRxParams.period = 104;
	m_timerRxParams.periodType = Timer_PeriodType_MICROSECS;
	m_timerRxParams.runMode = Timer_RunMode_CONTINUOUS;
	m_timerRxParams.startMode = Timer_StartMode_USER;
	m_timerRxParams.arg = (xdc_UArg)&m_uartHandle;
	m_timerRxHandle = Timer_create(Timer_ANY, (Timer_FuncPtr)SoftUARTRxTick, &m_timerRxParams, &m_timerRxEb);
	if(m_timerRxHandle == NULL)
	{
		return false;
	}

	Timer_start(m_timerRxHandle);

	return true;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
bool LED_CONTROL_RgbSend(tRGBControl* control)
{

	return true;
}

void LED_CONTROL_Test(uint16_t data)
{
	SoftSSIDataPut(&m_ssiHandle, (uint32_t)data);
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void SSICallback(void)
{
	//TODO
	m_intNumber++;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void UARTCallback(void)
{
	//TODO
	m_intNumber++;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void UARTGpioCallback(uint32_t arg)
{
	//TODO
}

///
/// \}
///
