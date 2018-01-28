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

    uint32_t cptFrameReceived;
    uint32_t cptFrameAborted;

}rx_data_t;

typedef struct tx_data
{
	//Fifo;
	cbFreeMsg_t cbFreeMsg;
	uint8_t *pMsg;
	uint8_t *pCurrCar;
	uint16_t size;

	bool (*cbNextState)(struct tx_data *pDataTx, uint8_t *pCar);
	uint32_t cptFrameSent;
	uint32_t cptFrameError;
}tx_data_t;

typedef struct
{
	bool isOpen;
	uint8_t linkNumber;
	rx_data_t rxMsg;
	tx_data_t txMsg;
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
static channel_t m_channels[MAX_CHANNELS_NUMBER];

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

static bool sendChar(void* pData, uint8_t *pCar);
static bool SendDLEStart(tx_data_t *pDataTx, uint8_t *pCar);
static bool SendSTX(tx_data_t *pDataTx, uint8_t *pCar);
static bool SendData(tx_data_t *pDataTx, uint8_t *pCar);
static bool SendDataDLE(tx_data_t *pDataTx, uint8_t *pCar);
static bool SendDLEEnd(tx_data_t *pDataTx, uint8_t *pCar);
static bool SendETX(tx_data_t *pDataTx, uint8_t *pCar);
static bool EndTX(tx_data_t *pDataTx, uint8_t *pCar);

static bool buildmessage(void *pData, uint8_t *pMsg, uint16_t maxSize);

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
									cbNotifyRx_t cbNotifyRx,
									void* pDataNotifyRx,
									cbAllocMsg_t cbAllocMsg,
									cbFreeMsg_t cbFreeMsg
									)
{
	SerialLinkConfig_t linkConf;
//	bool channelFound = false;
	uint8_t channelNumber = 0xFF;
	channel_t *pChannel = NULL;
	int i;

	//Find a free channels and open it
	for (i = 0; i < MAX_CHANNELS_NUMBER; ++i)
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

	pChannel->txMsg.cbFreeMsg = cbFreeMsg;
	pChannel->txMsg.cbNextState = EndTX;
	pChannel->txMsg.pMsg = NULL;

	pChannel->rxMsg.cbAllocMsg = cbAllocMsg;
	pChannel->rxMsg.cbFreeMsg = cbFreeMsg;
	pChannel->rxMsg.cbNotifyRx = cbNotifyRx;
	pChannel->rxMsg.pDataNotifyRX = pDataNotifyRx;

//	pChannel
	linkConf.baudrate = baurate;
	linkConf.dataSize = bitSize;
	linkConf.parity = parity;
	linkConf.stopBit = stopBit;
	linkConf.cbReception = charReceived;
	linkConf.pReceptionData = &pChannel->rxMsg;
	linkConf.cbTransmission = sendChar;
	linkConf.pTransmitionData = &pChannel->txMsg;

	if(SerialLink_Init(pChannel->linkNumber, &linkConf) != SERIAL_LINK_SUCCESS)
	{
		return false;
	}

	pChannel->isOpen = true;
	pChannel->rxMsg.charTreatment = waitStartDLE;
	return channelNumber;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
void SerialLinkFrameProtocole_Send(uint8_t channel, uint8_t *pMsg, uint16_t size)
{
	channel_t *pChannel = &m_channels[channel];

	if(!pChannel->isOpen)
	{
		return;
	}

	if(pChannel->txMsg.pMsg == NULL)
	{
		pChannel->txMsg.pMsg = pMsg;
		pChannel->txMsg.size = size;
		pChannel->txMsg.cbNextState = SendDLEStart;
		SerialLink_StartTX(pChannel->linkNumber);
	}
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
//        	pDataRx->cbAllocMsg(pDataRx->pMsg);
        	pDataRx->pMsg = pDataRx->cbAllocMsg();

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
        pDataRx->receivedChar++;
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


/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static bool sendChar(void* pData, uint8_t *pCar)
{
	tx_data_t *pDataTx = (tx_data_t*) pData;
	return ((tx_data_t*)pDataTx->cbNextState((tx_data_t*)pDataTx, pCar));

}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static bool SendDLEStart(tx_data_t *pDataTx, uint8_t *pCar)
{
	pDataTx->pCurrCar = pDataTx->pMsg;

	*pCar = DLE;
	pDataTx->cbNextState = SendSTX;

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
static bool SendSTX(tx_data_t *pDataTx, uint8_t *pCar)
{
	*pCar = STX;
	pDataTx->cbNextState = SendData;
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
static bool SendData(tx_data_t *pDataTx, uint8_t *pCar)
{
	*pCar = *pDataTx->pCurrCar;
	pDataTx->pCurrCar++;
	pDataTx->size--;

	if(*pCar == DLE)
	{
		//It is a DLE so double it
		pDataTx->cbNextState = SendDataDLE;
	}
	else
	{
		if(pDataTx->size == 0)
		{
			//No more character to send. End the frame
			pDataTx->cbNextState = SendDLEEnd;
		}
	}

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
static bool SendDataDLE(tx_data_t *pDataTx, uint8_t *pCar)
{
	*pCar = DLE;

	if(pDataTx->size == 0)
	{
		//No more characters to send
		pDataTx->cbNextState = SendDLEEnd;
	}
	else
	{
		pDataTx->cbNextState = SendData;
	}

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
static bool SendDLEEnd(tx_data_t *pDataTx, uint8_t *pCar)
{
	*pCar = DLE;
	pDataTx->cbNextState = SendETX;
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
static bool SendETX(tx_data_t *pDataTx, uint8_t *pCar)
{
	*pCar = ETX;

	pDataTx->pMsg = NULL;
	//End of frame
	pDataTx->cptFrameSent++;
	pDataTx->cbNextState = EndTX;

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
static bool EndTX(tx_data_t *pDataTx, uint8_t *pCar)
{
	return false;
}


/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static bool buildmessage(void *pData, uint8_t *pMsg, uint16_t maxSize)
{
	bool ret = true;
	tx_data_t *pDataTx = (tx_data_t *)pData;


	return ret;
}
///
/// \}
///
