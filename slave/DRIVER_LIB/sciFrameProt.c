////////////////////////////////////////////////////////////////////////////////
/// @addtogroup SCI_FRAME_PROT_M
/// @{
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// @file
/// Protocole de trame sur liaison serie
///
/// Gestion du protocole de type trame sur liaison serie
/// Il s'agit d'un protocole trame :
/// Chaque trame commence par la sequence <DLE><STX>
/// et se termine par la sequence <DLE><ETX>
/// Les donnes sont transmises entre ces deux sequences
/// Tout caractere <DLE> present dans les donnees est double
///     => remplace par <DLE><DLE>
/// \n \n <b>Copyright ADENEO SA</b>
////////////////////////////////////////////////////////////////////////////////

#define SCI_FRAME_PROT_M    // Pour gerer le prototype ET la definition des variables
                        // exportees du module

////////////////////////////////////////////////////////////////////////////////
//                                                                      Includes
////////////////////////////////////////////////////////////////////////////////
// D'abord les include standard (Chaine de compilation / Standard Adeset / Librairie)
#include "std_emb.h"
#include "os_wrapper.h"
#include "fifo.h"
#include "sci.h"
#include "sciFrameProt.h"
#include "emb_mem.h"

////////////////////////////////////////////////////////////////////////////////
//                                                               Defines & Types
////////////////////////////////////////////////////////////////////////////////

/// Nombre max de canaux utilisant le protocole trame sur LS
#define SCI_FRAME_PROT_NB_MAX_CHANNELS  FATSW_SCI_NB_CHANNEL

/// Nombre max de msg en attente dans les fifos d'emission
#define SCI_FRAME_PROT_NB_MAX_TX_MSG    5

// valeurs des caracteres de controle utilises dans le protocole
#define STX             0x02
#define ETX             0x03
#define DLE             0x10

typedef struct {
    struct FIFO_Elem_t fifo_elem;
    uint16_t      size;       ///< taille du message a emettre
    uint8_t*      pMsg;       ///< pointeur sur le message a emettre
} T_TX_MSG;

// Definition donnees et gestion emission
struct DATA_TX {
    struct FIFO_t fifoTx;           ///< FIFO des messages a emettre
    T_CALLBACK_SCI_FREE_MSG pCallBackFree; ///< callback pour liberer le buffer qui vient d'etre emis
    ///< Compteurs de debug
    uint16_t cptTxTrames;     ///< Nb de trames emises
};
typedef struct DATA_TX T_DATA_TX;

// Definition donnees et gestion reception
struct DATA_RX {
    void (*pFctStateRx)(struct DATA_RX* pDataRx, uint8_t car);
                            ///< pointeur sur la fct appelee en fonction
                            ///< de l'automate de reception
    uint16_t      maxSize;    ///< taille max de donnees que l'on peut recevoir
    T_CALLBACK_SCI_ALLOC_MSG pCallBackAlloc;
                            ///< callback pour allouer un nouveau buffer
    T_CALLBACK_SCI_NOTIFY_RX pCallBackNotifyRx;
                            ///< callback a appeler pour notifier la reception
                            ///< d'un message de maintenance au module qui va le traiter
    void*       pDataNotifyRx;///< Pointeur sur des donnees utilisateurs passees
                            ///< comme parametre de la callback
    uint8_t*      pMsg;       ///< Pointeur sur le message alloue
    uint8_t*      pCurrCar;   ///< Pointeur sur le caractere courant dans le buffer
    uint16_t      size;       ///< Taille recue
    // Compteurs de debug
    uint8_t  cptErrRxSTX;     ///< Nb d'erreurs sur attente STX
    uint8_t  cptErrRxAlloc;   ///< Nb d'erreurs allocation buffer
    uint8_t  cptErrRxOvf;     ///< Nb d'erreurs overflow (trame trop grande en reception)
    uint8_t  cptErrRxAbandon; ///< Nb d'abandons (reception d'un nouveau debut de trame et abandon de la trame en cours de constitution
    uint8_t  cptErrRxDLE;     ///< Nb d'erreurs DLE (car != de ETX,STX et DLE apres un DLE)
    uint8_t  cptErrSci;       ///< Nb d'erreurs bas niveau
    uint16_t cptRxTrames;     ///< Nb de trames recues
};
typedef struct DATA_RX T_DATA_RX;

typedef struct {
    bool        bOpen;      ///< le canal est-il ouvert
    uint8_t     noSCI;      ///< No de la ls utilisee pour ce canal
    T_DATA_TX   dataTx;     ///< donnees pour gerer l'emission
    T_DATA_RX   dataRx;     ///< donnees pour gerer la reception
} T_DATA_CHANNEL;


////////////////////////////////////////////////////////////////////////////////
//                                                                     Variables
////////////////////////////////////////////////////////////////////////////////
// Note : OBLIGATOIREMENT static


/// FIFO d'elements libres
static struct FIFO_t m_fifoTxFree;

/// Elements qui seront chaines dans la FIFO free lors de l'init
static T_TX_MSG m_arrTxMsg[SCI_FRAME_PROT_NB_MAX_TX_MSG];

/// Channels data
static T_DATA_CHANNEL m_arrChannel[SCI_FRAME_PROT_NB_MAX_CHANNELS];


////////////////////////////////////////////////////////////////////////////////
//                                             Prototypes des fonctions internes
////////////////////////////////////////////////////////////////////////////////
// Note : OBLIGATOIREMENT static
static int TxBuildMsg(void *pData, uint8_t *ptDst, int maxSize);
static void RxPutRdCar(void* pDataRx, uint8_t car);
static void NotifySciErr(void* pDataErr, uint8_t err);

// pour gerer l'automate de reception
static void RxWaitForDLEStart              (T_DATA_RX* pDataRx, uint8_t car);
static void RxWaitForSTX                   (T_DATA_RX* pDataRx, uint8_t car);
static void RxWaitForDataWithoutPreviousDLE(T_DATA_RX* pDataRx, uint8_t car);
static void RxWaitForDataWithPreviousDLE   (T_DATA_RX* pDataRx, uint8_t car);



////////////////////////////////////////////////////////////////////////////////
//                                                           Fonctions exportees
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
/// initialisation du module
///
////////////////////////////////////////////////////////////////////////////////
void SCI_FRAME_PROT_Init(void)
{
    // On initialise toutes les structures (bOpen = false et cpt tous a 0)
    EMB_memset(&m_arrChannel[0], 0, sizeof(m_arrChannel));

    // on chaine les elements du tableau dans la fifo d'elements libres
    FIFO_InitFromArray(&m_fifoTxFree, (uint8_t* )&m_arrTxMsg[0], sizeof(m_arrTxMsg[0]), SCI_FRAME_PROT_NB_MAX_TX_MSG);
}


////////////////////////////////////////////////////////////////////////////////
/// Ouverture d'un canal de maintenance sur une LS
///
/// \param maxSize : Taille maximum des donnees dans le message (utilisee en reception
///                  pour ne pas deborder du message)
/// \param pCallBackAlloc : Callback que le driver appelle pour obtenir un nouveau message
///                          lorsqu'il traite un debut de reception
/// \param pCallBackFree : Callback que le driver appelle pour liberer un message a la fin
///                          de l'emission
/// \param pCallBackNotifyRx : Callback que le driver appelle pour notifier la reception d'un
///                      message de maintenance a l'appelant
/// ATTENTION : ces callbacks sont susceptibles d'etre appelees dans une
/// ISR => verifier la sequence d'appel pour voir dans quel contexte les
/// fonctions seront appelees
/// \param pDataNotifyRx pointeur sur des donnees utilisateur qui seront repassees a
///                      l'utilisateur lors de l'appel de la callback
/// \param noSCI     : No de la LS sur laquelle ouvrir le canal de maintenance
/// \param vitesse   : Vitesse (cf sci.h)
/// \param donnee    : Nb de bits de donnee (cf sci.h)
/// \param bStop     : Stop bits (cf sci.h)
/// \param parite    : Parite (cf sci.h)
///
/// \return         No de canal obtenu (0xff si pas bon)
///
////////////////////////////////////////////////////////////////////////////////
uint8_t SCI_FRAME_PROT_Open( uint16_t                maxSize
                         , T_CALLBACK_SCI_ALLOC_MSG  pCallBackAlloc
                         , T_CALLBACK_SCI_FREE_MSG   pCallBackFree
                         , T_CALLBACK_SCI_NOTIFY_RX  pCallBackNotifyRx
                         , void*                 pDataNotifyRx
                         , uint8_t               noSCI
                         , T_SCI_BAUD_RATE       baudRate
                         , T_SCI_DATA_BITS       dataBits
                         , T_SCI_STOP_BITS       stopBits
                         , T_SCI_PARITY          parity
                         )
{
    T_DATA_CHANNEL *pChannel = NULL;
    T_SCI_PARAM paramSCI;
    T_SCI_ERR sciErr;

    if (   (NULL == pCallBackAlloc)
        || (NULL == pCallBackFree)
        || (NULL == pCallBackNotifyRx)
       )
    {
//        SRV_DBG(0);
        return SCI_FRAME_PROT_INVALID_CHANNEL;
    }

    if ((noSCI < SCI_FRAME_PROT_NB_MAX_CHANNELS) && (!m_arrChannel[noSCI].bOpen))
    {
        pChannel = &m_arrChannel[noSCI];
        pChannel->bOpen = true;
    }
    else
    {
        // Canal deja ouvert ou invalid => echec
        return SCI_FRAME_PROT_INVALID_CHANNEL;
    }

    // Ok un canal trouve, on l'initialise
    pChannel->noSCI = noSCI;

    // Initialisation des donnees pour la reception
    pChannel->dataRx.pFctStateRx  = RxWaitForDLEStart; // attente DLE de debut de trame
    pChannel->dataRx.pMsg         = NULL;              // pas de buffer encore alloue
    pChannel->dataRx.maxSize           = maxSize;
    pChannel->dataRx.pCallBackAlloc    = pCallBackAlloc;
    pChannel->dataRx.pCallBackNotifyRx = pCallBackNotifyRx;
    pChannel->dataRx.pDataNotifyRx     = pDataNotifyRx;

    // Initialisation des donnees pour l'emission
    FIFO_Init(&(pChannel->dataTx.fifoTx));        // Initialisation de la FIFO d'emission
    pChannel->dataTx.pCallBackFree = pCallBackFree;

    // Initialisation du canal utilise
    paramSCI.baudRate = baudRate;
    paramSCI.dataBits = dataBits;
    paramSCI.stopBits = stopBits;
    paramSCI.parity   = parity;
    paramSCI.pFctPutc = RxPutRdCar;  // callback de signalisation d'un caractere recu
    paramSCI.pDataPutc= &(pChannel->dataRx);
    paramSCI.pFctErr = NotifySciErr; // fonction appelee pour signaler une erreur
    paramSCI.pDataErr= &(pChannel->dataRx);
    paramSCI.pFctBMsg = TxBuildMsg; // callback de construction du message a emettre
    paramSCI.pDataBMsg= &(pChannel->dataTx);

    sciErr = SCI_Init(noSCI, &paramSCI);
    if (SCI_OK != sciErr)
    {
        // On libere le canal
//        SRV_DBG(sciErr);
        pChannel->bOpen = false;
        return SCI_FRAME_PROT_INVALID_CHANNEL;
    }
    // On retourne l'indice du canal alloue
    return noSCI;
}



////////////////////////////////////////////////////////////////////////////////
/// Emission d'un message de maintenance sur la liaison serie
/// Si une emission est en cours, le buffer est stocke dans une FIFO
/// Sinon, l'automate d'emission est initialise et l'emission lancee
/// Dans tous les cas, le buffer sera libere par le module a la fin
/// de l'emission
///
/// \param noChannel: canal sur lequel envoyer les donnees
/// \param pMsg     : pointeur sur le buffer a emettre
/// \param size     : taille des donnees a emettre
///
/// \retval false Si le buffer n'a pas pu être mit dans la FIFO: Le buffer n'a pas été libéré
////////////////////////////////////////////////////////////////////////////////
bool SCI_FRAME_PROT_Send(uint8_t noChannel, uint8_t* pMsg, uint16_t size)
{
    bool ok = true;
    T_DATA_CHANNEL *pChannel = &m_arrChannel[noChannel];
    T_TX_MSG *pTxMsg;

    if ((noChannel >= SCI_FRAME_PROT_NB_MAX_CHANNELS) || (!pChannel->bOpen))
    {
        // Canal inexistant ou non ouvert
//        SRV_DBG(0);
        return false;
    }

    pTxMsg = (T_TX_MSG *)FIFO_Get(&m_fifoTxFree);
    if (NULL == pTxMsg)
    {
//        SRV_DBG(noChannel);
        ok = false;
    }
    else
    {
        pTxMsg->size = size;
        pTxMsg->pMsg = pMsg;
        FIFO_Insert(&(pChannel->dataTx.fifoTx), (struct FIFO_Elem_t *)pTxMsg);
        SCI_StartTx(pChannel->noSCI);
    }

    return ok;
}


////////////////////////////////////////////////////////////////////////////////
//                                                           Fonctions internes
////////////////////////////////////////////////////////////////////////////////

//******************************************************************************
//******************************************************************************
//                                       FONCTIONS LIEES A L'AUTOMATE D'EMISSION
//******************************************************************************
//******************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// Fonction appelee par le driver SCI pour construire le message a emettre
///
/// \return  The length of the message, 0 if there is no message to send
////////////////////////////////////////////////////////////////////////////////
static int TxBuildMsg(void *pData, uint8_t *ptDst, int maxSize)
{
    int lenBuiltMsg = -1;
    T_DATA_TX* pDataTx = pData;
    T_TX_MSG *pTxMsg;
    uint8_t *pMsg;
    uint8_t *ptSrc;
    int nbDLE;
    int sizeRaw;
    int i;

    while (lenBuiltMsg < 0)
    {
        pTxMsg = (T_TX_MSG *)FIFO_Get(&(pDataTx->fifoTx));
        if (NULL != pTxMsg)
        {
            sizeRaw = pTxMsg->size;
            pMsg = pTxMsg->pMsg;

            // Free the message
            FIFO_Insert(&m_fifoTxFree, &(pTxMsg->fifo_elem));

            // Count the number of DLE
            nbDLE = 0;
            ptSrc = pMsg;

            for (i = 0; i < sizeRaw; ++i)
            {
                if (*ptSrc == DLE)
                {
                    ++nbDLE;
                }
                ++ptSrc;
            }

            // Compute the length of the final message
            lenBuiltMsg = sizeRaw + 4 + nbDLE;

            // Check that the message fit into the buffer
            if (lenBuiltMsg <= maxSize)
            {
                ptSrc = pMsg;

                *ptDst++ = DLE;
                *ptDst++ = STX;

                for (i = 0; i < sizeRaw; ++i)
                {
                    if (*ptSrc == DLE)
                    {
                        *ptDst++ = DLE;
                    }
                    *ptDst++ = *ptSrc++;
                }

                *ptDst++ = DLE;
                *ptDst = ETX;
            }
            else
            {
                // Message too big, continue...
                lenBuiltMsg = -1;
//                SRV_DBG(sizeRaw);
            }

            pDataTx->pCallBackFree(pMsg);
        }
        else
        {
            // No more message
            lenBuiltMsg = 0;
        }
    }

    return lenBuiltMsg;
}

//******************************************************************************
//******************************************************************************
//                                     FONCTIONS LIEES A L'AUTOMATE DE RECEPTION
//******************************************************************************
//******************************************************************************

////////////////////////////////////////////////////////////////////////////////
/// Fonction appelee par le driver SCI pour signaler une erreur
///
/// \param  err : masque indiquant les erreurs signalees
////////////////////////////////////////////////////////////////////////////////
static void NotifySciErr(void* pDataErr, uint8_t err)
{
    T_DATA_RX* pDataRx = (T_DATA_RX*)pDataErr;
    // une erreur est signalee, on ne se pose pas de question, on abandonne
    // la trame en cours de reception
    (void)err;

    pDataRx->pFctStateRx = RxWaitForDLEStart;
    pDataRx->cptErrSci++;
}

////////////////////////////////////////////////////////////////////////////////
/// Fonction appelee par le driver SCI dans l'ISR RX pour signaler
/// l'arrivee d'un nouveau caractere
///
/// \param  car : caractere recu
////////////////////////////////////////////////////////////////////////////////
static void RxPutRdCar(void* pDataRx, uint8_t car)
{
    // appel de la fonction suivant l'etat de l'automate de reception
    ((T_DATA_RX* )pDataRx)->pFctStateRx((T_DATA_RX* )pDataRx, car);
}


////////////////////////////////////////////////////////////////////////////////
/// Fonction de l'automate de reception qui attend le DLE de la sequence de debut
/// d'une trame
///
/// \param  car : caractere recu
////////////////////////////////////////////////////////////////////////////////
static void RxWaitForDLEStart(T_DATA_RX* pDataRx, uint8_t car)
{
    if (DLE == car)
    {
        // Ok, on attend maintenant un STX
        pDataRx->pFctStateRx = RxWaitForSTX;
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Fonction de l'automate de reception qui attend le STX de la sequence de debut
/// d'une trame
///
/// \param  car : caractere recu
////////////////////////////////////////////////////////////////////////////////
static void RxWaitForSTX(T_DATA_RX* pDataRx, uint8_t car)
{
    if (STX == car)
    {
        // Ok, on a eu un STX, on commence une nouvelle trame
        // On commence par demander un buffer si on n'en a pas deja un
        if (NULL == pDataRx->pMsg)
        {
            pDataRx->pMsg = pDataRx->pCallBackAlloc();
        }
        if (pDataRx->pMsg)
        {
            // Ok, on a bien un buffer
            pDataRx->size     = 0;
            pDataRx->pCurrCar = (uint8_t* ) pDataRx->pMsg;
            pDataRx->pFctStateRx = RxWaitForDataWithoutPreviousDLE;
        }
        else
        {
            // penurie de message, on ignore la trame en cours
            // et on se remet en attente d'un DLE de debut
            pDataRx->pFctStateRx = RxWaitForDLEStart;
            pDataRx->cptErrRxAlloc++;
        }
    }
    else
    {
        // pas le bon caractere, on se remet en attente du DLE de debut
        pDataRx->pFctStateRx = RxWaitForDLEStart;
        pDataRx->cptErrRxSTX++;
    }
}


////////////////////////////////////////////////////////////////////////////////
/// Fonction de l'automate de reception qui qui traite les caracteres recus
/// apres que le debut d'une trame (<DLE><STX>) ait ete detecte, le caratere
/// precedemment decode n'etant pas un <DLE>
///
/// \param  car : caractere recu
////////////////////////////////////////////////////////////////////////////////
static void RxWaitForDataWithoutPreviousDLE(T_DATA_RX* pDataRx, uint8_t car)
{
    // pas de caractere DLE le coup precedent
    if (DLE == car)
    {
        // on memorise que l'on a recu un DLE (pour le traiter au prochain
        // caractere
        pDataRx->pFctStateRx = RxWaitForDataWithPreviousDLE;
    }
    else
    {
        // Sinon, on stocke le caractere et on avance dans le buffer
        pDataRx->size++;
        if (pDataRx->size <= pDataRx->maxSize)
        {
            // pas de debordement
            *pDataRx->pCurrCar = car;
            pDataRx->pCurrCar++;
        }
        else
        {
            // Pas assez de place pour stocker la trame, on l'ignore
            pDataRx->pFctStateRx = RxWaitForDLEStart;
            pDataRx->cptErrRxOvf++;
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
/// Fonction de l'automate de reception qui traite le caractere recu
/// apres un DLE (le marqueur de debut d'une trame <DLE><STX> ayant deja
/// ete reconnu)
///
/// \param  car : caractere recu
////////////////////////////////////////////////////////////////////////////////
static void RxWaitForDataWithPreviousDLE(T_DATA_RX* pDataRx, uint8_t car)
{
    // le caractere precedent etait un DLE
    // pour le coup suivant
    pDataRx->pFctStateRx = RxWaitForDataWithoutPreviousDLE;
    switch (car)
    {
    case STX:
        // sequence de debut d'une nouvelle trame => on abandonne
        // celle en cours de constitution
        pDataRx->size     = 0;
        pDataRx->pCurrCar = (uint8_t* ) pDataRx->pMsg;
        pDataRx->cptErrRxAbandon++;
        break;
    case ETX:
        // Fin de la trame
        pDataRx->cptRxTrames++;
        // On la transmet au module qui va traiter (appel de la callback)
        pDataRx->pCallBackNotifyRx(pDataRx->pDataNotifyRx, pDataRx->pMsg, pDataRx->size);
        // On indique qu'on n'a plus de buffer
        pDataRx->pMsg = NULL;
        // On se remet en attente du debut d'une nouvelle trame
        pDataRx->pFctStateRx = RxWaitForDLEStart;
        break;
    case DLE:
        // DLE double, on stocke donc un DLE et on avance dans le buffer
        pDataRx->size++;
        if (pDataRx->size <= pDataRx->maxSize)
        {
            // pas de debordement
            *pDataRx->pCurrCar = DLE;
            pDataRx->pCurrCar++;
        }
        else
        {
            // Pas assez de place pour stocker la trame, on l'ignore
            pDataRx->pFctStateRx = RxWaitForDLEStart;
            pDataRx->cptErrRxOvf++;
        }
        break;
    default:
        // ce n'est pas bon, on abandonne la trame en cours et on
        // se remet en attente d'un debut de trame
        pDataRx->pFctStateRx = RxWaitForDLEStart;
        pDataRx->cptErrRxDLE++;
        break;
    }
}

///
/// @}
///
