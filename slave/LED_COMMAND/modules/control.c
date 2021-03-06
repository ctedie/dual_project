/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup control
/// \{
/// \author Dev
///
///	\brief
///
///
/// \file
/// \brief Fichier source du module control
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <main.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_hibernate.h"
#include "inc/hw_ints.h"

#include "driverlib/sysctl.h"
#include "driverlib/rom.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"

#include "control.h"
#include "common.h"
#include "exchange.h"

#include "serial_link.h"
#include "serial_link_frame_protocol.h"

/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

#define MAX_NB_MSG	4

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	bool isUsed;
	tExchangeMsg msg;
}tbuffMsg;


/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////

static uint8_t m_channelNumber = NULL;
static tbuffMsg m_arrMsg[MAX_NB_MSG];

static tRGBControl m_RGBStatus = {0, 0, 0, 0};


/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////

static uint8_t* AllocMsg(void);
static void FreeMsg(void *pMsg);
static void process_data(void* pData, uint8_t *pMsg, uint16_t size);

/////////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////////

bool Control_init(void)
{
	uint8_t i;
	//Init the buffer array
	for (i = 0; i < MAX_NB_MSG; ++i)
	{
		m_arrMsg[i].isUsed = false;
	}

	   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	   GPIOPinConfigure(GPIO_PB0_U1RX);
	   GPIOPinConfigure(GPIO_PB1_U1TX);
	   GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	   SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

	m_channelNumber = SerialLinkFrameProtocoleInit(SERIAL2,
												   B115200,
												   BIT_8,
												   PARITY_NONE,
												   STOP_BIT_1,
												   process_data,
												   &m_channelNumber,
												   AllocMsg,
												   FreeMsg
												   );


	return true;

}

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static uint8_t* AllocMsg(void)
{
	uint8_t i;

	for (i = 0; i < MAX_NB_MSG; ++i)
	{
		if(m_arrMsg[i].isUsed == false)
		{
			m_arrMsg[i].isUsed = true;
			return (uint8_t*)&(m_arrMsg[i].msg);
		}

	}
	return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void FreeMsg(void *pMsg)
{
	uint8_t i;

	for (i = 0; i < MAX_NB_MSG; ++i)
	{
		if(&m_arrMsg[i].msg == pMsg)
		{
			m_arrMsg[i].isUsed = false;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void process_data(void* pData, uint8_t *pMsg, uint16_t size)
{
	tExchangeMsg* message = (tExchangeMsg*)pMsg;

	switch (message->header.commandID)
	{
		case COMMAND_SET_RGB_CONTROL:
			if(message->header.datasize == sizeof(tRGBControl))
			{
				RGB_SetColor((tRGBControl*)message->req.data);
			}

			break;
		case COMMAND_GET_RGB_CONTROL:
			if(message->header.datasize == sizeof(tRGBControl))
			{

			}
			break;
		case COMMAND_GET_STATUS:
			if(message->header.datasize == sizeof(tSlaveStatus))
			{

			}

			break;
		case COMMAND_GET_DATE_TIME:
			if(message->header.datasize == sizeof(tDateTime))
			{

			}
			break;
		case COMMAND_UPDATE_DATE_TIME:

			break;
		default:
			break;
	}

}

///
/// \}
///
