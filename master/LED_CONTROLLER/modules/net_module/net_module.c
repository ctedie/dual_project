/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup net_module
/// \{
/// \author Dev
///
///	\brief 
///
///
/// \file
/// \brief Fichier source du module net_module
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include "system.h"

/* NDK BSD support */
#include <sys/socket.h>

#include "net_module.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define NET_STACKSIZE	1024
#define NUMTCPWORKERS 1

/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
Task_Handle netTaskHandle;
Bool bEndTCPTask = false;

/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
static void NET_TaskBody(UArg arg0, UArg arg1);
static void NET_processConnection(UArg arg0, UArg arg1);

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
Bool NET_init(void)
{
	Task_Params taskParams;

	Task_Params_init(&taskParams);
//    taskParams.arg0 = (UArg)clientfd;:
    taskParams.stackSize = NET_STACKSIZE;
    taskParams.priority = 1;
    netTaskHandle = Task_create((Task_FuncPtr)NET_TaskBody, &taskParams, NULL);
    if (netTaskHandle == NULL)
    {
        UARTprintf("netOpenHook: Failed to create tcpHandler Task\n");
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
static void NET_TaskBody(UArg arg0, UArg arg1)
{
	int                status;
	int                client;
	int                server;
	struct sockaddr_in localAddr;
	struct sockaddr_in clientAddr;
	int                optval;
	int                optlen = sizeof(optval);
	int          addrlen = sizeof(clientAddr);
	Task_Handle        taskHandle;
	Task_Params        taskParams;
	Error_Block        eb;

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == -1) {
		System_printf("Error: socket not created.\n");
//		NDK_shutdown(server, SHUT_RDWR);
	    System_flush();
	}
	else
	{
		memset(&localAddr, 0, sizeof(localAddr));
		localAddr.sin_family = AF_INET;
		localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		localAddr.sin_port = htons(1000);

		status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
		if (status == -1) {
			System_printf("Error: bind failed.\n");
			close(server);
	//		goto shutdown;
		}

		status = listen(server, NUMTCPWORKERS);
		if (status == -1) {
			System_printf("Error: listen failed.\n");
			close(server);
	//		goto shutdown;
		}

		optval = 1;
		if (setsockopt(server, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen) < 0) {
			System_printf("Error: setsockopt failed\n");
			close(server);
	//		goto shutdown;
		}

		while ((client = accept(server, (struct sockaddr *)&clientAddr, &addrlen)) != -1)
		{

	//		System_printf("tcpHandler: Creating thread clientfd = %d\n", clie);

			/* Init the Error_Block */
			Error_init(&eb);

			/* Initialize the defaults and set the parameters. */
			Task_Params_init(&taskParams);
			taskParams.arg0 = (UArg)client;
			taskParams.stackSize = 1280;
			taskHandle = Task_create((Task_FuncPtr)NET_processConnection, &taskParams, &eb);
			if (taskHandle == NULL) {
				System_printf("Error: Failed to create new Task\n");
				close(server);
			}

			/* addrlen is a value-result param, must reset for next accept call */
			addrlen = sizeof(clientAddr);
		}
	}

	close(server);

//
//	System_printf("Error: accept failed.\n");
//
//shutdown:
//	if (server > 0) {
//		close(server);
//	}

}

uint64_t cptTask = 0;
/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void NET_processConnection(UArg arg0, UArg arg1)
{
	SOCKET client = (SOCKET)arg0;


	while(!bEndTCPTask)
	{
		cptTask++;
		Task_sleep(1000);
	}

	NDK_shutdown(client, SHUT_RDWR);
}


///
/// \}
///
