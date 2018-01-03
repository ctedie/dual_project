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

#include "common.h"
#include "exchange.h"
#include "serial_link.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

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
				.uartNb = UART0_BASE,
				.baudrate = B115200,
				.dataSize = BIT_8,
				.parity = PARITY_NONE,
				.cbReception = NULL,
				.cbTransmission = NULL,
				.initOK = false
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
uint8_t SerialLink_Init(SerialLinkNumber_t link)
{
	uint8_t ret = 0;

	uint32_t config = 0;

	switch (m_SerialLinkList[link].dataSize)
	{
		case BIT_7:
			config |= UART_CONFIG_WLEN_7;
			break;
		case BIT_8:
			config |= UART_CONFIG_WLEN_8;
			break;
		default:
			config |= UART_CONFIG_WLEN_8;
			break;
	}

	switch (m_SerialLinkList[link].parity)
	{
		case PARITY_NONE:
			config |= UART_CONFIG_PAR_NONE;
			break;
		default:
			config |= UART_CONFIG_PAR_NONE;
			break;
	}

	switch (m_SerialLinkList[link].stopBit)
	{
		case STOP_BIT_1:
			config |= UART_CONFIG_STOP_ONE;
			break;
		default:
			config |= UART_CONFIG_STOP_ONE;
			break;
	}

	m_SerialLinkList[link].initOK = true;


    SysCtlPeripheralEnable(m_UARTPeriph[link]);

	UARTConfigSetExpClk((uint32_t)m_SerialLinkList[link].uartNb,
						(uint32_t)SysCtlClockGet(),
						(uint32_t)m_SerialLinkList[link].baudrate,
						(uint32_t)config
						);

    if(m_SerialLinkList[link].cbReception != NULL)
    {
    	m_SerialLinkList[link].cbReception = testCallback;

    	UARTFIFOLevelSet(m_SerialLinkList[link].uartNb, UART_FIFO_TX1_8, UART_FIFO_RX1_8);
    	m_SerialLinkList[link].pReceptionArg = &receivedChar;

    	UARTIntDisable(m_SerialLinkList[link].uartNb, 0xFFFFFF);
    	UARTIntRegister(m_UARTInt[link], m_IntFuncTable[link]);
    	UARTIntEnable(m_SerialLinkList[link].uartNb, UART_INT_RX | UART_INT_RT);
    	UARTIntClear(m_SerialLinkList[link].uartNb, UART_INT_RX | UART_INT_RT);
    	IntEnable(m_UARTInt[link]);
    }

    UARTEnable(m_SerialLinkList[link].uartNb);
//    m_SerialLinkList[uartNb].pReceptionArg =


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
uint8_t SerialLink_Write(uint8_t sLink, uint8_t *pBuffer, const uint16_t size)
{
	uint8_t ret = 0;
	uint16_t i;


	if(m_SerialLinkList[sLink].initOK == false)
	{

		ret = 1;
	}

	if(ret == 0)
	{
		for (i = 0; i < size; ++i)
		{

			UARTCharPut(m_SerialLinkList[sLink].uartNb, pBuffer[i]);
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
uint8_t SerialLink_Read(uint8_t sLink, uint8_t *pBuffer, const uint16_t size)
{
	uint8_t ret = 0;
	uint16_t i;


	if(m_SerialLinkList[sLink].initOK == false)
	{

		ret = 1;
	}

	if(ret == 0)
	{
		for (i = 0; i < size; ++i)
		{

			pBuffer[i] = UARTCharGet(m_SerialLinkList[sLink].uartNb);
		}
	}

	return ret;
}


/////////////////////////////////////////////////////////////////////////////////
// Private functions
/////////////////////////////////////////////////////////////////////////////////

void serialLinkIntHandler0(void)
{
	generalIntHandler(UART0_BASE);
}

void serialLinkIntHandler1(void)
{
	generalIntHandler(UART1_BASE);
}

void serialLinkIntHandler2(void)
{
	generalIntHandler(UART2_BASE);
}


static void generalIntHandler(uint32_t uartNb)
{
	uint32_t intStatus;
	char car;

	intStatus = UARTIntStatus(uartNb, true);
	UARTIntClear(uartNb, intStatus);

	if(intStatus & (UART_INT_RX | UART_INT_RT))
	{
		if(m_SerialLinkList[uartNb].cbReception != NULL)
		{
		receivedChar = (char)UARTCharGetNonBlocking(m_SerialLinkList[uartNb].uartNb);
		m_SerialLinkList[uartNb].cbReception(m_SerialLinkList[uartNb].uartNb, &receivedChar);
		}
	}
	else if(intStatus == UART_INT_TX)
	{
		if(m_SerialLinkList[uartNb].cbTransmission != NULL)
		{
			m_SerialLinkList[uartNb].cbTransmission(m_SerialLinkList[uartNb].uartNb, m_SerialLinkList[uartNb].pTransmissionArg);
		}

	}
	else
	{
//		UARTCharGet(m_SerialLinkList[uartNb].uartNb);
	}



}

static uint16_t cptReceivedChar;
void testCallback(uint8_t uartNb, void* pArg)
{
	char* character = pArg;




}

///
/// \}
///
