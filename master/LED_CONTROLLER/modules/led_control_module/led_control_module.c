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

#include "system.h"

#include "serial_link.h"
#include "serial_link_frame_protocol.h"
#include "common.h"
#include "exchange.h"
#include "led_control_module.h"

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define MAX_NB_MSG	4

/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

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
static uint16_t m_cptErr;

static Timer_Handle timer0;
/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
static uint8_t* AllocMsg(void);
static void FreeMsg(void *pMsg);
static void process_data(void* pData, uint8_t *pMsg, uint16_t size);

static void LED_CONTROL_Test(UArg arg0);

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

	uint8_t i;
	uint32_t key;
	//Init the buffer array
	for (i = 0; i < MAX_NB_MSG; ++i)
	{
		m_arrMsg[i].isUsed = false;
	}

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);
    GPIOPinConfigure(GPIO_PC4_U7RX);
    GPIOPinConfigure(GPIO_PC5_U7TX);
    GPIOPinTypeUART(GPIO_PORTC_BASE, GPIO_PIN_4 | GPIO_PIN_5);

    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART7);

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

	timer0 = Timer_create(Timer_ANY, LED_CONTROL_Test, NULL, NULL);
	  key = Hwi_disable();
	Timer_setPeriodMicroSecs(timer0, 5000); //5ms
	  Timer_start(timer0);
	  Hwi_restore(key);


//	Timer_start(timer0);
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
	tExchangeMsg* pMsg;

	pMsg = (tExchangeMsg*)AllocMsg();

	pMsg->header.commandID = COMMAND_SET_RGB_CONTROL;
	pMsg->header.datasize = sizeof(tRGBControl);
	memcpy(pMsg->req.data, control, sizeof(tRGBControl));

	SerialLinkFrameProtocole_Send(m_channelNumber, (uint8_t*)pMsg, (sizeof(tExchangeHeader) + sizeof(tRGBControl)));

	return true;
}

static tRGBControl m_RGBControl = {0, 0, 0, 0};
enum stateColor
{
	RED,
	GREEN,
	BLUE
};
enum stateColor nextColor = RED;
void LED_CONTROL_Test(UArg arg0)
{
	switch (nextColor)
	{
	case RED:
		if(m_RGBControl.red < 0xFFFF)
		{
			m_RGBControl.red++;
		}
		else
		{
			m_RGBControl.red = 0;
			nextColor = GREEN;
		}
		break;
	case GREEN:
		if(m_RGBControl.green < 0xFFFF)
		{
			m_RGBControl.green++;
		}
		else
		{
			m_RGBControl.green = 0;
			nextColor = BLUE;
		}

		break;
	case BLUE:
		if(m_RGBControl.blue < 0xFFFF)
		{
			m_RGBControl.blue++;
		}
		else
		{
			m_RGBControl.blue = 0;
			nextColor = RED;
		}

		break;
		default:
			break;
	}
	LED_CONTROL_RgbSend(&m_RGBControl);

}


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
			if(message->ans.status != STATUS_OK)
			{
				m_cptErr++;
			}
			break;
		case COMMAND_GET_RGB_CONTROL:
			if(message->header.datasize == sizeof(tRGBControl))
			{
				memcpy(&m_RGBStatus, message->ans.data, sizeof(tRGBControl));
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
