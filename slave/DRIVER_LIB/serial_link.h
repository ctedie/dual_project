/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup serial_link
/// \{
///
///
/// \file
/// \brief Fichier en-tete du module serial_link
///
///
///
/////////////////////////////////////////////////////////////////////////////////

// Define to prevent recursive inclusion
#ifndef __SERIAL_LINK_H_
/// \cond IGNORE_FOLLOWING
#define __SERIAL_LINK_H_
/// \endcond

#ifdef __cplusplus
extern "C"
{
#endif
/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
/////////////////////////////////////////////////////////////////////////////////
// Exported types
/////////////////////////////////////////////////////////////////////////////////

typedef void (*SerialLinkCallback)(uint8_t sLink, void* pArg);


typedef enum
{
	SERIAL1 = 0,
	SERIAL2,
	SERIAL3,

	NB_SERIAL
}SerialLinkNumber_t;

typedef enum
{
	B9600 = 9600,
	B19200 = 19200,
	B115200 = 115200,
	B4600800 = 4600800
}SerialLinkSpeed_t;

typedef enum
{
	BIT_7,
	BIT_8,
	BIT_9
}SerialLinkDataSize_t;

typedef enum
{
	PARITY_NONE,
	PARITY_EVEN,
	PARITY_ODD
}SerialLinkParity_t;

typedef enum
{
	STOP_BIT_1,
	STOP_BIT_2
}SerialLinkStopBit_t;


typedef struct
{
	uint32_t uartNb;
	SerialLinkSpeed_t baudrate;
	SerialLinkDataSize_t dataSize;
	SerialLinkParity_t parity;
	SerialLinkStopBit_t stopBit;
	SerialLinkCallback cbReception;
	void* pReceptionArg;
	SerialLinkCallback cbTransmission;
	void* pTransmissionArg;
	bool initOK;
}SerialLink_t;
/////////////////////////////////////////////////////////////////////////////////
// Exported constants
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Exported functions
/////////////////////////////////////////////////////////////////////////////////

uint8_t SerialLink_Init(uint8_t sLink);
uint8_t SerialLink_Read(uint8_t sLink, uint8_t *pBuffer, const uint16_t size);
uint8_t SerialLink_Write(uint8_t sLink, uint8_t *pBuffer, uint16_t size);

void serialLinkIntHandler0(void);
void serialLinkIntHandler1(void);
void serialLinkIntHandler2(void);

#ifdef __cplusplus
}
#endif

#endif /* SCI_LIB_H_ */

///
/// \}
///
