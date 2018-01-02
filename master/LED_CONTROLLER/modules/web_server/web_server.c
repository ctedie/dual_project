/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup web_server
/// \{
/// \author Dev
///
///	\brief
///
///
/// \file
/// \brief Fichier source du module web_server
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>

#include "system.h"

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* NDK BSD support */
#include <sys/socket.h>

#include "web_server.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define HTTP_PORT	80
/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
static Task_Handle        webtaskHandle;
static Task_Params        webtaskParams;
static Error_Block        weberrorBlock;
/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
static void WebServer_TaskBody(UArg arg0, UArg arg1);
static void serve(int clientfd);
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
void WebServer_Init(void)
{

    /* Init the Error_Block */
    Error_init(&weberrorBlock);

    /* Initialize the defaults and set the parameters. */
    Task_Params_init(&webtaskParams);
    webtaskParams.stackSize = 4096;

    webtaskHandle = Task_create((Task_FuncPtr)WebServer_TaskBody, &webtaskParams, &weberrorBlock);
    if (webtaskHandle == NULL)
    {
        System_printf("Error: Failed to create new Task\n");
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
static void WebServer_TaskBody(UArg arg0, UArg arg1)
{
    int                status;
    int                clientfd;
    int                server;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    int                optval;
    int                optlen = sizeof(optval);
    int          addrlen = sizeof(clientAddr);

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) {
        System_printf("Error: socket not created.\n");
        goto shutdown;
    }


    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(HTTP_PORT);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        System_printf("Error: bind failed.\n");
        goto shutdown;
    }

    status = listen(server, 3);
    if (status == -1) {
        System_printf("Error: listen failed.\n");
        goto shutdown;
    }

    optval = 1;
    if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
        System_printf("Error: setsockopt failed\n");
        goto shutdown;
    }

    while ((clientfd = accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1)
    {

		serve(clientfd);

		close(clientfd);
	}

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);


    System_printf("Error: accept failed.\n");

shutdown:
    if (server > 0) {
        close(server);
    }
}

static void serve(int clientfd)
{
    int  bytesRcvd;
    int  bytesSent;
    char buffer[1024];

    System_printf("serve: start clientfd = 0x%x\n", clientfd);

    bytesRcvd = recv(clientfd, buffer, 1024, 0);
    System_printf("%s\n", buffer);
    UARTprintf("\n");
    UARTprintf("#####################################\n");
    UARTprintf("##          HTTP REQUEST           ##\n");
    UARTprintf("#####################################\n");
    UARTprintf("\n");
    UARTprintf("%s\n\n\n", buffer);
    System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);

    close(clientfd);
}

///
/// \}
///
