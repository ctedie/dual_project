/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup serial_link_frame_protocole
/// \{
/// \author Dev
///
/// \brief
///
///
/// \file
/// \brief Fichier source du module serial_link_frame_protocole
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"

#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"
#include "driverlib/uart.h"

#include "serial_link.h"
#include "serial_link_frame_protocol.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

//typedef void (*cbCharTreatment_t)(rx_data_t *pDataRx, uint8_t car);

typedef struct rx_data
{
    void (*charTreatment)(struct rx_data *pDataRx, uint8_t car);

    uint8_t *pMsg;
    uint8_t *receivedChar;
    uint16_t msgSize;

    cbNotifyRx_t cbNotifyRx;
    void* pDataNotifyRX;

    cbAllocMsg_t cbAllocMsg;
    cbFreeMsg_t cbFreeMsg;

}rx_data_t;

typedef struct
{
	bool isOpen;
	uint8_t linkNumber;
	rx_data_t rxMsg;
//	tx_data_t txMsg;
}channel_t;
/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define STX             0x02
#define ETX             0x03
#define DLE             0x10

#define MAX_CHANNELS_NUMBER		4
/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
channel_t m_channels[MAX_CHANNELS_NUMBER];

/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
static void charReceived(void *pDataRx, uint8_t car);

static void waitStartDLE(rx_data_t *pDataRx, uint8_t car);
static void waitSTX(rx_data_t *pDataRx, uint8_t car);
static void waitDataWithoutDLE(rx_data_t *pDataRx, uint8_t car);
static void waitDataWithDLE(rx_data_t *pDataRx, uint8_t car);

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
uint8_t SerialLinkFrameProtocoleInit(SerialLinkNumber_t link,
									SerialLinkSpeed_t baurate,
									SerialLinkDataSize_t bitSize,
									SerialLinkParity_t parity,
									SerialLinkStopBit_t stopBit,
									cbNotifyRx_t pcbNotifyRx,
									void* pDataNotifyRx,
									cbAllocMsg_t cbAllocMsg,
									cbFreeMsg_t cbFreeMsg
									)
{
	SerialLinkConfig_t linkConf;
//	bool channelFound = false;
	uint8_t channelNumber = 0xFF;
	channel_t *pChannel = NULL;


	//Find a free channels and open it
	for (int i = 0; i < MAX_CHANNELS_NUMBER; ++i)
	{
		if(m_channels[i].isOpen == false)
		{
			m_channels[i].linkNumber = link;
			channelNumber = i;
			pChannel = &m_channels[i];
			break;
		}
	}

	// No free channel
	if(channelNumber == 0xFF)
	{
		return channelNumber;
	}


	linkConf.baudrate = baurate;
	linkConf.dataSize = bitSize;
	linkConf.parity = parity;
	linkConf.stopBit = stopBit;
	linkConf.cbReception = charReceived;
	linkConf.pReceptionArg = &pChannel->rxMsg;

	pChannel->rxMsg.cbAllocMsg = cbAllocMsg;
	pChannel->rxMsg.cbFreeMsg = cbFreeMsg;

	if(SerialLink_Init(pChannel->linkNumber, &linkConf) != SERIAL_LINK_SUCCESS)
	{
		return false;
	}

	pChannel->isOpen = true;
	return channelNumber;
}

/////////////////////////////////////////////////////////////////////////////////
// Local Functions
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void charReceived(void *pData, uint8_t car)
{
    rx_data_t* pDataRx = (rx_data_t*)pData;
    pDataRx->charTreatment(pDataRx, car);
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void waitStartDLE(rx_data_t *pDataRx, uint8_t car)
{
    if(car == DLE)
    {
        pDataRx->charTreatment = waitSTX;
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
static void waitSTX(rx_data_t *pDataRx, uint8_t car)
{
    if(car == STX)
    {
        if(pDataRx->pMsg == NULL) //No previous buffer so try to alloc one
        {
            //TODO Alloc pMsg
        	pDataRx->cbAllocMsg(pDataRx->pMsg);
        }

        if(pDataRx->pMsg)
        {
            //Initialise the message informations
            pDataRx->msgSize = 0;
            pDataRx->receivedChar =  pDataRx->pMsg;
            pDataRx->charTreatment = waitDataWithoutDLE;
        }
        else //No available buffer
        {
            pDataRx->charTreatment = waitStartDLE;
        }
    }
    else
    {
        pDataRx->charTreatment = waitStartDLE;
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
static void waitDataWithoutDLE(rx_data_t *pDataRx, uint8_t car)
{
    if(car == DLE)
    {
        pDataRx->charTreatment = waitDataWithDLE;
    }
    else
    {
        *pDataRx->receivedChar = car;
        pDataRx->receivedChar++;
        pDataRx->msgSize++;
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
static void waitDataWithDLE(rx_data_t *pDataRx, uint8_t car)
{

    pDataRx->charTreatment = waitDataWithoutDLE;

    if(car == DLE)
    {
        //DLE double Stocker 1 DLE en tant que data
        *pDataRx->receivedChar = car;
        pDataRx->msgSize++;
    }
    else if(car == STX)
    {
        // Nouvelle trame : Abandon de la precedente et début d'une nouvelle
        pDataRx->msgSize = 0;
        pDataRx->receivedChar = pDataRx->pMsg;
    }
    else if(car == ETX)
    {
        // Fin de la trame : Notifier la couche supérieure
        pDataRx->charTreatment = waitStartDLE;
        pDataRx->cbNotifyRx(pDataRx->pDataNotifyRX, pDataRx->pMsg, pDataRx->msgSize);

        //Liberer la memoire msg
        pDataRx->pMsg = NULL;
    }
    else
    {
        //Erreur : Abandonner la trame

        pDataRx->charTreatment = waitStartDLE;

        //Liberer la memoire msg
        pDataRx->cbFreeMsg(pDataRx->pMsg);
    }

}

///
/// \}
///
