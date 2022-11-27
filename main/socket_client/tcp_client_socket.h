/*************** INTERFACE CHANGE LIST **************************************    
*    
*    Date       	Software Version    Initials        Description    
*  27 Nov 2022         0.0.1           STYLVN      Interface Created.    
*    
*****************************************************************************/    
/* @file:   tcp_client_socket.h   
 * @brief:  This header contains   
 */ 
#ifndef MAIN_SOCKET_CLIENT_TCP_CLIENT_SOCKET_H_
#define MAIN_SOCKET_CLIENT_TCP_CLIENT_SOCKET_H_

/******************************************************************************
* Includes
*******************************************************************************/


/******************************************************************************
* Preprocessor Constants
*******************************************************************************/
#define TCP_HOST_IP_ADDR 							"192.168.100.49"
#define TCP_HOST_PORT								7777
#define TCP_CLIENT_SOCKET_DEFAULT_SEND_MGS			"Hello there! \r\n"
#define TCP_CLIENT_SOCKET_DEFAULT_MSG_LEN			(100)

#define TCP_CLIENT_SOCKET_TASK_STACK_SIZE			(5*1024)
#define TCP_CLIENT_SOCKET_TASK_PRIORITY				(tskIDLE_PRIORITY + 1)
#define TCP_CLIENT_SOCKET_TASK_INDEX				(1)

/******************************************************************************
* Configuration Constants
*******************************************************************************/


/******************************************************************************
* Macros
*******************************************************************************/


/******************************************************************************
* Typedefs
*******************************************************************************/


/******************************************************************************
* Variables
*******************************************************************************/
extern TaskHandle_t xTCP_socket_handler;


/******************************************************************************
* Function Prototypes
*******************************************************************************/
void tcp_client_socket_init();
void tcp_client_socket_task(void* param);

#endif /* MAIN_SOCKET_CLIENT_TCP_CLIENT_SOCKET_H_ */
