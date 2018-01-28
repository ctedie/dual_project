/*
 * control.c
 *
 *  Created on: 28 janv. 2018
 *      Author: Dev
 */


#include <RGBproject.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "control.h"
#include "common.h"
#include "exchange.h"

#include "serial_link.h"
#include "serial_link_frame_protocol.h"

#define MAX_NB_MSG	4


typedef struct
{
	bool isUsed;
	tExchangeMsg msg;
}tbuffMsg;



static uint8_t m_channelNumber = NULL;
static tbuffMsg m_arrMsg[MAX_NB_MSG];

static tRGBControl m_RGBStatus = {0, 0, 0, 0};



static uint8_t* AllocMsg(void);
static void FreeMsg(void *pMsg);
static void process_data(void* pData, uint8_t *pMsg, uint16_t size);


bool Control_init(void)
{
	uint8_t i;
	//Init the buffer array
	for (i = 0; i < MAX_NB_MSG; ++i)
	{
		m_arrMsg[i].isUsed = false;
	}

	m_channelNumber = SerialLinkFrameProtocoleInit(SERIAL1,
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
