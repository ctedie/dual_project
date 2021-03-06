/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup serial_link
/// \{
/// \author Dev
///
///	\brief
///
///
/// \file
/// \brief Fichier source du module serial_link
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
#include "driverlib/udma.h"

#include "common.h"
#include "exchange.h"
#include "serial_link.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

typedef struct
{
	uint32_t uartBase;

	SerialLinkReceiveCallback cbReception;
	void* pReceptionData;
	SerialLinkTransmitCallback cbTransmission;
	void* pTransmitionData;
	SerialLinkEndOfTransmitionCallback cbEndOfTransmition;
	void* pEndOfTransmitionArg;
	bool initOK;

	uint32_t _intFlags;
}SerialLink_t;

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
SerialLink_t m_SerialLinkList[NB_SERIAL] =
{
		{
				.uartBase = UART0_BASE,
		},

		{
				.uartBase = UART1_BASE,
		}
};




static const uint32_t m_UARTPeriph[NB_SERIAL] =
{
		SYSCTL_PERIPH_UART0, SYSCTL_PERIPH_UART1, SYSCTL_PERIPH_UART2
};

static const uint32_t m_UARTInt[NB_SERIAL] =
{
    INT_UART0, INT_UART1, INT_UART2
};

typedef void (*pIntFunction)(void);
static const pIntFunction m_IntFuncTable[] =
{
        serialLinkIntHandler0,
        serialLinkIntHandler1,
        serialLinkIntHandler2
};

/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////

static void generalIntHandler(uint32_t uartNb);
void testCallback(uint8_t uartNb, void* pArg);
static void disableTxInterrupt(uint32_t uartNb);
static void enableTxInterrupt(uint32_t uartNb);
/////////////////////////////////////////////////////////////////////////////////
// Functions
/////////////////////////////////////////////////////////////////////////////////
static char receivedChar;
/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
SerialLinkReturn_t SerialLink_Init(SerialLinkNumber_t link, SerialLinkConfig_t *pConfig)
{
	SerialLinkReturn_t ret = SERIAL_LINK_SUCCESS;

	uint32_t config = 0;
	uint32_t intFlags = 0;
	uint32_t fifoLevelsFlags = 0;

	if(pConfig == NULL)
	{
		//Error
		return SERIAL_LINK_BAD_CONFIG;
	}

	m_SerialLinkList[link].cbReception = pConfig->cbReception;
	m_SerialLinkList[link].pReceptionData = pConfig->pReceptionData;
	m_SerialLinkList[link].cbTransmission = pConfig->cbTransmission;
	m_SerialLinkList[link].pTransmitionData = pConfig->pTransmitionData;
	m_SerialLinkList[link].cbEndOfTransmition = pConfig->cbEndOfTransmition;
	m_SerialLinkList[link].pEndOfTransmitionArg = pConfig->pEndOfTransmitionArg;


	switch (pConfig->dataSize)
	{
		case BIT_7:
			config |= UART_CONFIG_WLEN_7;
			break;
		case BIT_8:
			config |= UART_CONFIG_WLEN_8;
			break;
		default:
			return SERIAL_LINK_BAD_CONFIG;
	}

	switch (pConfig->parity)
	{
		case PARITY_NONE:
			config |= UART_CONFIG_PAR_NONE;
			break;
		default:
			return SERIAL_LINK_BAD_CONFIG;
	}

	switch (pConfig->stopBit)
	{
		case STOP_BIT_1:
			config |= UART_CONFIG_STOP_ONE;
			break;
		default:
			return SERIAL_LINK_BAD_CONFIG;
	}



    SysCtlPeripheralEnable(m_UARTPeriph[link]);

	UARTConfigSetExpClk((uint32_t)m_SerialLinkList[link].uartBase,
						(uint32_t)SysCtlClockGet(),
						(uint32_t)pConfig->baudrate,
						(uint32_t)config
						);

    if(m_SerialLinkList[link].cbReception != NULL)
    {
    	m_SerialLinkList[link]._intFlags = (UART_INT_RX | UART_INT_RT);
    }


    //DMA configure



	UARTFIFOLevelSet(m_SerialLinkList[link].uartBase, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
	UARTTxIntModeSet(m_SerialLinkList[link].uartBase, UART_TXINT_MODE_EOT);

	UARTIntDisable(m_SerialLinkList[link].uartBase, 0xFFFFFF);
	UARTIntRegister(m_SerialLinkList[link].uartBase, m_IntFuncTable[link]);
	UARTIntEnable(m_SerialLinkList[link].uartBase, m_SerialLinkList[link]._intFlags);
	UARTIntClear(m_SerialLinkList[link].uartBase, m_SerialLinkList[link]._intFlags);
	IntEnable(m_UARTInt[link]);
	UARTFIFOEnable(m_SerialLinkList[link].uartBase);

    UARTEnable(m_SerialLinkList[link].uartBase);
//    m_SerialLinkList[uartNb].pReceptionArg =

	m_SerialLinkList[link].initOK = true;

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
SerialLinkReturn_t SerialLink_StartTX(uint8_t sLink)
{
	uint8_t car;

	if(sLink > 4)
	{
		return SERIAL_LINK_BAD_ID;
	}

	if(m_SerialLinkList[sLink].initOK == false)
	{
		return SERIAL_LINK_NOT_INIT;
	}

	while(UARTSpaceAvail(m_SerialLinkList[sLink].uartBase))
	{
		//Send the first characters
		if(m_SerialLinkList[sLink].cbTransmission(m_SerialLinkList[sLink].pTransmitionData, &car))
		{
			UARTCharPutNonBlocking(m_SerialLinkList[sLink].uartBase, car);

		}
		else
		{
			break;
		}
	}

	enableTxInterrupt(sLink);
	return SERIAL_LINK_SUCCESS;

}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
SerialLinkReturn_t SerialLink_Write(uint8_t sLink, uint8_t *pBuffer, const uint16_t size)
{
	uint8_t ret = SERIAL_LINK_SUCCESS;
	uint16_t i;


	if(m_SerialLinkList[sLink].initOK == false)
	{

		ret = SERIAL_LINK_NOT_INIT;
	}

	if(ret == SERIAL_LINK_SUCCESS)
	{
		for (i = 0; i < size; ++i)
		{
			//TODO Non blocking
			UARTCharPut(m_SerialLinkList[sLink].uartBase, pBuffer[i]);
		}
	}

	return ret;
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
SerialLinkReturn_t SerialLink_Read(uint8_t sLink, uint8_t *pBuffer, const uint16_t size)
{
	uint8_t ret = SERIAL_LINK_SUCCESS;
	uint16_t i;


	if(m_SerialLinkList[sLink].initOK == false)
	{

		ret = SERIAL_LINK_NOT_INIT;
	}

	if(ret == SERIAL_LINK_SUCCESS)
	{
		for (i = 0; i < size; ++i)
		{

			pBuffer[i] = UARTCharGet(m_SerialLinkList[sLink].uartBase);
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////
// Private functions
/////////////////////////////////////////////////////////////////////////////////

void serialLinkIntHandler0(void)
{
	generalIntHandler(SERIAL1);
}

void serialLinkIntHandler1(void)
{
	generalIntHandler(SERIAL2);
}

void serialLinkIntHandler2(void)
{
	generalIntHandler(SERIAL3);
}


static void generalIntHandler(uint32_t uartNb)
{
	uint32_t intStatus;
	uint8_t car;
	uint8_t txCar;

	intStatus = UARTIntStatus(m_SerialLinkList[uartNb].uartBase, true);
	UARTIntClear(m_SerialLinkList[uartNb].uartBase, intStatus);

	if(intStatus & (UART_INT_RX | UART_INT_RT))
	{
		if(m_SerialLinkList[uartNb].cbReception != NULL)
		{
			if(UARTCharsAvail(m_SerialLinkList[uartNb].uartBase))
            {
				car = (uint8_t)UARTCharGetNonBlocking(m_SerialLinkList[uartNb].uartBase);
				m_SerialLinkList[uartNb].cbReception(m_SerialLinkList[uartNb].pReceptionData, car);
            }
		}
	}
	else if(intStatus & UART_INT_TX)
	{
		if(m_SerialLinkList[uartNb].cbTransmission != NULL)
		{
			while(UARTSpaceAvail(m_SerialLinkList[uartNb].uartBase))
			{
				//Send the first characters
				if(m_SerialLinkList[uartNb].cbTransmission(m_SerialLinkList[uartNb].pTransmitionData, &car))
				{
					UARTCharPutNonBlocking(m_SerialLinkList[uartNb].uartBase, car);

				}
				else
				{
					break;
				}
			}

		}

	}
	else
	{
//		UARTCharGet(m_SerialLinkList[uartNb].uartNb);
	}



}

static void disableTxInterrupt(uint32_t uartNb)
{
	m_SerialLinkList[uartNb]._intFlags &= ~UART_INT_TX;
	UARTIntDisable(m_SerialLinkList[uartNb].uartBase, UART_INT_TX);
}

static void enableTxInterrupt(uint32_t uartNb)
{
	m_SerialLinkList[uartNb]._intFlags |= UART_INT_TX;
	UARTIntEnable(m_SerialLinkList[uartNb].uartBase, UART_INT_TX);
}

static uint16_t cptReceivedChar;
void testCallback(uint8_t uartNb, void* pArg)
{
	char* character = pArg;

}


///
/// \}
///
