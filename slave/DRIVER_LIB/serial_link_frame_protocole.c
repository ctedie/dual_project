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
///
typedef void (*cbCharTreatment_t)(struct rx_data *pDataRx, uint8_t car);

typedef struct rx_data
{
    cbCharTreatment_t charTreatment;

    uint8_t *pMsg;
    uint8_t *receivedChar;
    uint16_t msgSize;

    cbNotifyRx_t cbNotifyRx;
    void* pDataNotifyRX;

}rx_data_t;

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define STX             0x02
#define ETX             0x03
#define DLE             0x10
/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////

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
bool SerialLinkFrameProtocoleInit(void)
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
// Local Functions
/////////////////////////////////////////////////////////////////////////////////
///
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
    rx_data_t* pDataRx = &pData;
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
        pDataRx->charTreatment = (cbCharTreatment_t)waitSTX;
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
        }

        if(pDataRx->pMsg)
        {
            //Initialise the message informations
            pDataRx->msgSize = 0;
            pDataRx->receivedChar =  pDataRx->pMsg;
            pDataRx->charTreatment = (cbCharTreatment_t)waitDataWithoutDLE;
        }
        else //No available buffer
        {
            pDataRx->charTreatment = (cbCharTreatment_t)waitStartDLE;
        }
    }
    else
    {
        pDataRx->charTreatment = (cbCharTreatment_t)waitStartDLE;
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
        pDataRx->charTreatment = (cbCharTreatment_t)waitDataWithDLE;
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

    pDataRx->charTreatment = (cbCharTreatment_t)waitDataWithoutDLE;

    if(car == DLE)
    {
        //DLE double Stocker 1 DLE en tant que data
        *pDataRx->receivedChar = car;
        pDataRx->msgSize++;
    }
    else if(car == STX)
    {
        // Nouvelle trame : Abandon de la precedente et d�but d'une nouvelle
        pDataRx->msgSize = 0;
        pDataRx->receivedChar = pDataRx->pMsg;
    }
    else if(car == ETX)
    {
        // Fin de la trame : Notifier la couche sup�rieure
        pDataRx->charTreatment = (cbCharTreatment_t)waitStartDLE;
        pDataRx->cbNotifyRx(pDataRx->pDataNotifyRX, pDataRx->pMsg, pDataRx->msgSize);

        //Liberer la memoire msg
        pDataRx->pMsg = NULL;
    }
    else
    {
        //Erreur : Abandonner la trame

        pDataRx->charTreatment = (cbCharTreatment_t)waitStartDLE;

        //Liberer la memoire msg
        pDataRx->pMsg = NULL;
    }

}

///
/// \}
///
