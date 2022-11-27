/*************** INTERFACE CHANGE LIST **************************************    
*    
*    Date       	Software Version    Initials        Description    
*  27 Nov 2022         0.0.1      		STYLVN      Interface Created.    
*    
*****************************************************************************/    
/* @file: 	tcp_client_socket.c   
 * @brief: 	This module contains   
 */  

/******************************************************************************    
* Includes    
*******************************************************************************/    
#include "task_common.h"

#include "lwip/sockets.h"
#include "lwip/err.h"

#include "tcp_client_socket.h"


/******************************************************************************    
* Module Preprocessor Constants    
*******************************************************************************/    
#define MODULE_NAME			"TCP_SOCKET_CLIENT: "
 
/******************************************************************************    
* Module Preprocessor Macros    
*******************************************************************************/    
    

/******************************************************************************    
* Module Typedefs    
*******************************************************************************/    
    

/******************************************************************************    
* Module Variable Definitions    
*******************************************************************************/    
    

/******************************************************************************    
* Function Prototypes    
*******************************************************************************/    


/******************************************************************************    
* Function Definitions    
*******************************************************************************/    
void tcp_client_socket_init()
{
	xTaskCreate(TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].TaskCodePtr,
				TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].TaskName,
				TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].StackDepth,
				TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].ParametersPtr,
				TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].TaskPriority,
				TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].TaskHandle);

	assert(NULL != TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].TaskHandle);
}

void tcp_client_socket_task(void* param)
{
	/* Init */
	ESP_LOGI(MODULE_NAME, "/************************ TCP SOCKET CLIENT STARTED ************************/\r\n");

	bool is_terminate_conn = false;
	char send_msg[TCP_CLIENT_SOCKET_DEFAULT_MSG_LEN] = TCP_CLIENT_SOCKET_DEFAULT_SEND_MGS;
	char recv_msg[TCP_CLIENT_SOCKET_DEFAULT_MSG_LEN] = {0};

	struct sockaddr_in dest_addr;
	dest_addr.sin_addr.s_addr = inet_addr(TCP_HOST_IP_ADDR);
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(TCP_HOST_PORT);


	while(!is_terminate_conn)
	{
		int client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
		if(client_socket < 0)
		{
			ESP_LOGE(MODULE_NAME, "Unable to create socket: errno %d \r\n", errno);
			continue;
		}
        ESP_LOGI(MODULE_NAME, "Socket created, connecting to %s:%d", TCP_HOST_IP_ADDR, TCP_HOST_PORT);

        int err = connect(client_socket, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in6));
		if (err != 0)
		{
			ESP_LOGE(MODULE_NAME, "Socket unable to connect: errno %d", errno);
			break;
		}
		ESP_LOGI(MODULE_NAME, "Successfully connected");

		while(1)
		{
			int err = send(client_socket, TCP_CLIENT_SOCKET_DEFAULT_SEND_MGS, strlen(TCP_CLIENT_SOCKET_DEFAULT_SEND_MGS), 0);
			if (err < 0)
			{
				ESP_LOGE(MODULE_NAME, "Error occurred during sending: errno %d", errno);
				break;
			}

			int len = recv(client_socket, recv_msg, sizeof(recv_msg) - 1, 0);
			if (len < 0)
			{
				// Error occurred during receiving
				ESP_LOGE(MODULE_NAME, "Recv failed: errno %d", errno);
				break;
			}
			else
			{
				// Data received
				recv_msg[len] = 0; // Null-terminate whatever we received and treat like a string
				ESP_LOGI(MODULE_NAME, "Received %d bytes from %s:", len, TCP_HOST_IP_ADDR);
				ESP_LOGI(MODULE_NAME, "%s", recv_msg);

				/* Trigger terminate event */
				if(recv_msg[0] == 'S')
				{
					is_terminate_conn = true;
				}
			}
			vTaskDelay(2000 / portTICK_PERIOD_MS);

		}

        if (client_socket != -1)
        {
            ESP_LOGE(MODULE_NAME, "Shutting down socket and restarting...");
            shutdown(client_socket, 0);
            close(client_socket);
        }
	}
	ESP_LOGI(MODULE_NAME, "Terminated connection");
	vTaskDelete(TasksTable[TCP_CLIENT_SOCKET_TASK_INDEX].TaskHandle);
	ESP_LOGI(MODULE_NAME, "TCP socket client task deleted \r\n");
	ESP_LOGI(MODULE_NAME, "/************************ TCP SOCKET CLIENT FINISHED ************************/\r\n");

}
