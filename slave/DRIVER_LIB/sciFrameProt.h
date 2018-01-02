////////////////////////////////////////////////////////////////////////////////
/// @addtogroup SCI_FRAME_PROT
/// @{
///
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @file 
/// Interfaces du module SCI_FRAME_PROT
///
///  \n \n <b>Copyright ADENEO SA</b>
////////////////////////////////////////////////////////////////////////////////


// Pour se proteger de la multi-inclusion : NOM_MODULE_H doit etre unique
#ifndef SCI_FRAME_PROT_H
#define SCI_FRAME_PROT_H


////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////
// Uniquement des includes projet : pas d'inclusion de fichier standard


// Pour gerer le prototypage et la declaration
#undef PUBLIC
#ifdef SCI_FRAME_PROT_M
#define PUBLIC
#else
#define PUBLIC extern
#endif



////////////////////////////////////////////////////////////////////////////////
//                                                      Defines & Types exportes
////////////////////////////////////////////////////////////////////////////////

#define SCI_FRAME_PROT_INVALID_CHANNEL 0xFF       // canal invalide


typedef uint8_t *(*T_CALLBACK_SCI_ALLOC_MSG)(void);
typedef void (*T_CALLBACK_SCI_FREE_MSG)(uint8_t *pMsg);
typedef void (*T_CALLBACK_SCI_NOTIFY_RX)(void *pData, uint8_t *pMsg, uint16_t size);


////////////////////////////////////////////////////////////////////////////////
//                                                           Variables exportees
////////////////////////////////////////////////////////////////////////////////
// Note : PREMOD est le prefixe du module en majuscule et court


////////////////////////////////////////////////////////////////////////////////
//                                                           Fonctions exportees
////////////////////////////////////////////////////////////////////////////////

PUBLIC void SCI_FRAME_PROT_Init(void);

PUBLIC uint8_t SCI_FRAME_PROT_Open( uint16_t                maxSize
                                , T_CALLBACK_SCI_ALLOC_MSG  pCallBackAlloc
                                , T_CALLBACK_SCI_FREE_MSG   pCallBackFree
                                , T_CALLBACK_SCI_NOTIFY_RX  pCallBackNotifyRx
                                , void                 *pDataNotifyRx
                                , uint8_t               noSCI
                                , T_SCI_BAUD_RATE       baudRate
                                , T_SCI_DATA_BITS       dataBits
                                , T_SCI_STOP_BITS       stopBits
                                , T_SCI_PARITY          parity
                                );

PUBLIC bool SCI_FRAME_PROT_Send(uint8_t noChannel, uint8_t *pMsg, uint16_t size);

#endif // SCI_FRAME_PROT_H

///
/// @}
///
