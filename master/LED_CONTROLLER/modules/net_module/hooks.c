/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup hooks
/// \{
/// \author Dev
///
///	\brief 
///
///
/// \file
/// \brief Fichier source du module hooks
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include "system.h"


#include <sys/socket.h>

#include "net_module.h"
/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define TCPPORT 1000

#define TCPHANDLERSTACK 1024
#define TCPPORT 1000

#define TCPPACKETSIZE 256
#define NUMTCPWORKERS 3

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
void netOpenHook()
{
    Task_Handle taskHandle;
    Task_Params taskParams;
    Error_Block eb;

//    /* Make sure Error_Block is initialized */
//    Error_init(&eb);
//
//    /*
//     *  Create the Task that farms out incoming TCP connections.
//     *  arg0 will be the port that this task listens to.
//     */
//    Task_Params_init(&taskParams);
//    taskParams.stackSize = TCPHANDLERSTACK;
//    taskParams.priority = 1;
//    taskParams.arg0 = TCPPORT;
//    taskHandle = Task_create((Task_FuncPtr)tcpHandler, &taskParams, &eb);
//    if (taskHandle == NULL) {
//        System_printf("netOpenHook: Failed to create tcpHandler Task\n");
//    }

//    System_flush();

//    UARTprintf("Net openned\n");
//    NET_init();

}


//Void tcpHandler(UArg arg0, UArg arg1)
//{
//    int                status;
//    int                clientfd;
//    int                server;
//    struct sockaddr_in localAddr;
//    struct sockaddr_in clientAddr;
//    int                optval;
//    int                optlen = sizeof(optval);
//    int          addrlen = sizeof(clientAddr);
//    Task_Handle        taskHandle;
//    Task_Params        taskParams;
//    Error_Block        eb;
//
//    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//    if (server == -1) {
//        System_printf("Error: socket not created.\n");
//        goto shutdown;
//    }
//
//
//    memset(&localAddr, 0, sizeof(localAddr));
//    localAddr.sin_family = AF_INET;
//    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
//    localAddr.sin_port = htons(arg0);
//
//    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
//    if (status == -1) {
//        System_printf("Error: bind failed.\n");
//        goto shutdown;
//    }
//
//    status = listen(server, NUMTCPWORKERS);
//    if (status == -1) {
//        System_printf("Error: listen failed.\n");
//        goto shutdown;
//    }
//
//    optval = 1;
//    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
//        System_printf("Error: setsockopt failed\n");
//        goto shutdown;
//    }
//
//    while ((clientfd =
//            accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1) {
//
//        System_printf("tcpHandler: Creating thread clientfd = %d\n", clientfd);
//
//        /* Init the Error_Block */
//        Error_init(&eb);
//
//        /* Initialize the defaults and set the parameters. */
//        Task_Params_init(&taskParams);
//        taskParams.arg0 = (UArg)clientfd;
//        taskParams.stackSize = 1280;
//        taskHandle = Task_create((Task_FuncPtr)tcpWorker, &taskParams, &eb);
//        if (taskHandle == NULL) {
//            System_printf("Error: Failed to create new Task\n");
//            close(clientfd);
//        }
//
//        /* addrlen is a value-result param, must reset for next accept call */
//        addrlen = sizeof(clientAddr);
//    }
//
//    System_printf("Error: accept failed.\n");
//
//shutdown:
//    if (server > 0) {
//        close(server);
//    }
//}


///
/// \}
///
