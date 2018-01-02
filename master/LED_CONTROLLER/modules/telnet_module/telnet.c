/////////////////////////////////////////////////////////////////////////////////
/// \addtogroup telnet
/// \{
/// \author Dev
///
///	\brief
///
///
/// \file
/// \brief Fichier source du module telnet
///
///
///
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////////////////////////
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "system.h"

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>

/* NDK BSD support */
#include <sys/socket.h>

#include "txt_lib.h"
#include "telnet.h"

/////////////////////////////////////////////////////////////////////////////////
// Private typedef
/////////////////////////////////////////////////////////////////////////////////

typedef int (*pfnCommand)(int clientfd, int argc, char *argv[]);

typedef struct
{
	const char *commandName;
	pfnCommand commandFct;
	const char *helpText;

}Command_t;
/////////////////////////////////////////////////////////////////////////////////
// Private define
/////////////////////////////////////////////////////////////////////////////////
#define TELNET_PORT	23
#define PROMPT	"LED_CONTROLLER> "
#define NB_MAX_ARG		8
/////////////////////////////////////////////////////////////////////////////////
// Private macro
/////////////////////////////////////////////////////////////////////////////////
#define NET_SEND()
/////////////////////////////////////////////////////////////////////////////////
// Private variables
/////////////////////////////////////////////////////////////////////////////////
static Task_Handle        telnetTaskHandle;
static Task_Params        telnetTaskParams;
static Error_Block        telnetErrorBlock;

static Timer_Handle timerTelnet;
static Semaphore_Handle semTelnet;

static char *g_ppcArgv[NB_MAX_ARG + 1];
static char g_NETBuffer[1024];
uint16_t g_bufferIndex = 0;
/////////////////////////////////////////////////////////////////////////////////
// Exported variables
/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////
// Private function prototypes
/////////////////////////////////////////////////////////////////////////////////
static void Telnet_TaskBody(UArg arg0, UArg arg1);
static void com_task(UArg arg0, UArg arg1);
static void cbPeriodicFunc(UArg arg0);
static int CommandProcess(int clientfd, char *pcCmdLine);

/* Command list */

static int command_help(int clientfd, int argc, char* argv[]);
static int command_exit(int clientfd, int argc, char* argv[]);



static Command_t g_psNetCmdTable[] =
{
		{"help"	, command_help,		"Print some help informations"},
		{"exit"	, command_exit,		"Exit the terminal"},
		{NULL, NULL, NULL}
};
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
void Telnet_Init(void)
{

//	timerTelnet = Timer_create(Timer_ANY, cbPeriodicFunc, NULL, NULL);
//
//    Timer_setPeriodMicroSecs(timerTelnet, 20000);
//
//
//
//    //Semaphore creation
//    semTelnet = Semaphore_create(0, NULL, NULL);

    /* Init the Error_Block */
    Error_init(&telnetErrorBlock);

    /* Initialize the defaults and set the parameters. */
    Task_Params_init(&telnetTaskParams);
    telnetTaskParams.stackSize = 2048;

    telnetTaskHandle = Task_create((Task_FuncPtr)Telnet_TaskBody, &telnetTaskParams, &telnetErrorBlock);
    if (telnetTaskHandle == NULL)
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
static void Telnet_TaskBody(UArg arg0, UArg arg1)
{
    int                status;
    int                clientfd;
    int                server;
    struct sockaddr_in localAddr;
    struct sockaddr_in clientAddr;
    int                optval;
    int                optlen = sizeof(optval);
    int          addrlen = sizeof(clientAddr);

    /* Parameters for the communication task */
    Task_Handle        taskHandle;
    Task_Params        taskParams;
    Error_Block        eb;

    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server == -1) {
        System_printf("Error: socket not created.\n");
        goto shutdown;
    }


    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    localAddr.sin_port = htons(TELNET_PORT);

    status = bind(server, (struct sockaddr *)&localAddr, sizeof(localAddr));
    if (status == -1) {
        System_printf("Error: bind failed.\n");
        goto shutdown;
    }

    status = listen(server, 1);
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

        System_printf("tcpHandler: Creating thread clientfd = %d\n", clientfd);

        /* Init the Error_Block */
        Error_init(&eb);

        /* Initialize the defaults and set the parameters. */
        Task_Params_init(&taskParams);
        taskParams.arg0 = (UArg)clientfd;
        taskParams.stackSize = 4096+2048;
        taskHandle = Task_create((Task_FuncPtr)com_task, &taskParams, &eb);
        if (taskHandle == NULL) {
            System_printf("Error: Failed to create new Task\n");
            close(clientfd);
        }

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);
    }

        /* addrlen is a value-result param, must reset for next accept call */
        addrlen = sizeof(clientAddr);


    System_printf("Error: accept failed.\n");

shutdown:
    if (server > 0) {
        close(server);
    }
}

static void com_task(UArg arg0, UArg arg1)
{
    int  bytesRcvd;
    int  bytesSent;
    int clientfd = (int)arg0;
    unsigned char buffer[1024];
    bool bContinue = true;

    memset(buffer, 0, 1024);
		System_printf("serve: start clientfd = 0x%x\n", clientfd);
//    	Semaphore_pend(semTelnet, BIOS_WAIT_FOREVER);
        while ((bytesRcvd = recv(clientfd, buffer, 1024, 0)) > 0)
        {

    		/* echo */
            bytesSent = send(clientfd, buffer, bytesRcvd, 0);
            if (bytesSent < 0 || bytesSent != bytesRcvd)
            {
                System_printf("Error: send failed.\n");
                break;
            }

            /* save to buffer */
			memcpy(g_NETBuffer + g_bufferIndex, buffer, bytesRcvd);
            g_bufferIndex += bytesRcvd;
            if((g_NETBuffer[g_bufferIndex-1] == '\n') || (g_NETBuffer[g_bufferIndex-1] == '\r'))
            {
            	g_NETBuffer[g_bufferIndex-1] = 0;
            	g_bufferIndex = 0;
            	CommandProcess(clientfd, g_NETBuffer);
            }
            else
            {

            }
            //Process cmd


        }

//		UARTprintf("%s\n\n\n", buffer);
		System_printf("tcpWorker stop clientfd = 0x%x\n", clientfd);



    close(clientfd);
}

/////////////////////////////////////////////////////////////////////////////////
/// \brief
///
/// \param
///
/// \return
///
/////////////////////////////////////////////////////////////////////////////////
static void cbPeriodicFunc(UArg arg0)
{
	Semaphore_post(semTelnet);

}


static int CommandProcess(int clientfd, char *pcCmdLine)
{
    char *pcChar;
    uint_fast8_t ui8Argc;
    bool bFindArg = true;
    bool bFindString = true;
    bool isStringArg = false;
    Command_t *psCmdEntry;

    //
    // Initialize the argument counter, and point to the beginning of the
    // command line string.
    //
    ui8Argc = 0;
    pcChar = pcCmdLine;

    //
    // Advance through the command line until a zero character is found.
    //
    while(*pcChar)
    {

    	// If there is a " or a ', it's a string argument
    	// Replace it by 0 and search for the end of the string
    	if((*pcChar == '\"') || (*pcChar == '\''))
    	{
    		//if we are searching the end of the string
    		if(isStringArg)
    		{
                *pcChar = 0;
				isStringArg = false;
				bFindArg = true;
    		}
    		else //The beginning of the string
    		{
    			*pcChar = 0;
				isStringArg = true;
    		}
    	}

        //
        // If there is a space, then replace it with a zero, and set the flag
        // to search for the next argument.
        //
    	else if((*pcChar == ' ') && (isStringArg == false))
        {
            *pcChar = 0;
            bFindArg = true;
        }

        //
        // Otherwise it is not a space, so it must be a character that is part
        // of an argument.
        //
        else
        {
            //
            // If bFindArg is set, then that means we are looking for the start
            // of the next argument.
            //
            if(bFindArg)
            {
                //
                // As long as the maximum number of arguments has not been
                // reached, then save the pointer to the start of this new arg
                // in the argv array, and increment the count of args, argc.
                //
                if(ui8Argc < NB_MAX_ARG)
                {
                    g_ppcArgv[ui8Argc] = pcChar;
                    ui8Argc++;
                    bFindArg = false;
                }

                //
                // The maximum number of arguments has been reached so return
                // the error.
                //
                else
                {
                    return(CMDLINE_TOO_MANY_ARGS);
                }
            }
        }

        //
        // Advance to the next character in the command line.
        //
        pcChar++;
    }

    //
    // If one or more arguments was found, then process the command.
    //
    if(ui8Argc)
    {
        //
        // Start at the beginning of the command table, to look for a matching
        // command.
        //
        psCmdEntry = &g_psNetCmdTable[0];

        //
        // Search through the command table until a null command string is
        // found, which marks the end of the table.
        //
        while(psCmdEntry->commandName)
        {
            //
            // If this command entry command string0 matches argv[0], then call
            // the function for this command, passing the command line
            // arguments.
            //
            if(!strcmp(g_ppcArgv[0], psCmdEntry->commandName))
            {
                return(psCmdEntry->commandFct(clientfd,ui8Argc, g_ppcArgv));
            }

            //
            // Not found, so advance to the next entry.
            //
            psCmdEntry++;
        }
    }

    //
    // Fall through to here means that no matching command was found, so return
    // an error.
    //
    return(CMDLINE_BAD_CMD);
}

static int command_help(int clientfd, int argc, char* argv[])
{
	char tmpBuff[1024];

	sprintf(tmpBuff, "This is the help command\n");
	send(clientfd, tmpBuff, strlen(tmpBuff), 0);

	return 0;
}

static int command_exit(int clientfd, int argc, char* argv[])
{
	char tmpBuff[1024];

	sprintf(tmpBuff, "This is the exit command\nNothig to do\n");
	send(clientfd, tmpBuff, strlen(tmpBuff), 0);

	return 0;
}
///
/// \}
///
