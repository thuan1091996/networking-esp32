/*************** INTERFACE CHANGE LIST **************************************    
*    
*    Date       	Software Version    Initials        Description    
*  27 Nov 2022         0.0.1      		Itachi      Interface Created.    
*    
*****************************************************************************/    
/* @file: 	http_client.c   
 * @brief: 	This module contains HTTP/HTTPS client example
 */  

/******************************************************************************    
* Includes    
*******************************************************************************/    
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include <esp_http_client.h>
#include <esp_crt_bundle.h>


#include "http_client.h"
#include "task_common/task_common.h"

   
/******************************************************************************    
* Module Preprocessor Constants    
*******************************************************************************/    
#define MODULE_NAME						"HTTP_CLIENT:"
#define HTTP_RESP_MAX_LEN				(512)
#define HTTPS_DEFAULT_RECV_LEN			(10)

#define HTTP_USE_CHUNKING				1
#define HTTP_USE_HTTP_REQUEST			1
#define HTTP_USE_HTTPS_REQUEST			1
#define HTTPS_USE_ESP_CERT_BUNDLE		1


/******************************************************************************    
* Module Preprocessor Macros    
*******************************************************************************/    
    

/******************************************************************************    
* Module Typedefs    
*******************************************************************************/    
typedef struct
{
	uint8_t* 	p_payload;
	uint32_t	payload_len;
}large_payload_t;


/******************************************************************************    
* Module Variable Definitions    
*******************************************************************************/    
TaskHandle_t xHTTP_handler = NULL;

#if !HTTPS_USE_ESP_CERT_BUNDLE
static const unsigned char wordtime_cert[] asm("_binary_worldtime_pem_start");
#endif /* End of HTTPS_USE_ESP_CERT_BUNDLE */


/******************************************************************************    
* Function Prototypes    
*******************************************************************************/    
    

/******************************************************************************    
* Function Definitions    
*******************************************************************************/
static esp_err_t https_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(MODULE_NAME, "HTTPS_EVENT_ERROR");
            break;

        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(MODULE_NAME, "HTTPS_EVENT_ON_CONNECTED");
        	ESP_LOGI(MODULE_NAME, "Client handler to: 0x%X", (uint32_t)evt->client);
            break;

        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(MODULE_NAME, "HTTPS_EVENT_HEADER_SENT");
            break;

        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(MODULE_NAME, "Received header data: %s: %s", evt->header_key, evt->header_value);
            break;

        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(MODULE_NAME, "HTTPS_EVENT_ON_DATA, len=%d", evt->data_len);
//			ESP_LOGI(MODULE_NAME, "HTTP response data: %s \r\n", (char*)evt->data);
        	/* Get data from event into response buffer */
            large_payload_t* recv_data = (large_payload_t*)evt->user_data;

            /* Alloc new buffer with appropriate size */
            recv_data->p_payload = realloc(recv_data->p_payload, recv_data->payload_len + evt->data_len + 1);
            assert(NULL != recv_data->p_payload);

            /* Move data to new buffer */
            memmove(&recv_data->p_payload[recv_data->payload_len], evt->data, evt->data_len);
            recv_data->payload_len += evt->data_len;

            /* Update NULL character */
            recv_data->p_payload[recv_data->payload_len] = 0;


			#if 0
			if (!esp_http_client_is_chunked_response(evt->client))
            {
                printf("[Response data]: %.*s \r\n", evt->data_len, (char*)evt->data);
            }
			#endif /* End of 0 */

            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(MODULE_NAME, "HTTPS_EVENT_ON_FINISH");
            break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(MODULE_NAME, "HTTPS_EVENT_DISCONNECTED");
            break;

    }
    return ESP_OK;
}

static esp_err_t http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {

        case HTTP_EVENT_ERROR:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ERROR");
            break;

        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ON_CONNECTED");
        	ESP_LOGI(MODULE_NAME, "Client handler to: 0x%X", (uint32_t)evt->client);
            break;

        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_HEADER_SENT");
            break;

        case HTTP_EVENT_ON_HEADER:
            // ESP_LOGE(MODULE_NAME, "Header length %d \r\n", evt->data_len); /* Header -> evt->data_len = 0 */
            ESP_LOGI(MODULE_NAME, "Received header data: %s: %s", (char*)evt->header_key, (char*)evt->header_value);
            break;

        case HTTP_EVENT_ON_DATA:
            /* Get data from event into response buffer */
            memcpy(evt->user_data, evt->data, evt->data_len);
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
			ESP_LOGI(MODULE_NAME, "HTTP response data: %s \r\n", (char*)evt->user_data);
            break;

        case HTTP_EVENT_ON_FINISH:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ON_FINISH");
            break;

        case HTTP_EVENT_DISCONNECTED:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_DISCONNECTED");
            break;

    }
    return ESP_OK;
}

void http_client_init()
{
	xTaskCreate(TasksTable[HTTP_TASK_INDEX].TaskCodePtr,
				TasksTable[HTTP_TASK_INDEX].TaskName,
				TasksTable[HTTP_TASK_INDEX].StackDepth,
				TasksTable[HTTP_TASK_INDEX].ParametersPtr,
				TasksTable[HTTP_TASK_INDEX].TaskPriority,
				TasksTable[HTTP_TASK_INDEX].TaskHandle);
	assert(NULL != TasksTable[HTTP_TASK_INDEX].TaskHandle);
	ESP_LOGI(MODULE_NAME, "HTTP task created \r\n");
}

void http_task(void* param)
{

#if HTTP_USE_HTTP_REQUEST
	/* HTTP request */
	ESP_LOGI(MODULE_NAME, "/************************ TESTING HTTP REQUEST************************/\r\n");
	http_client_req();
	ESP_LOGI(MODULE_NAME, "/************************ END TESTING HTTP REQUEST************************/\r\n");
#endif /* End of HTTP_USE_HTTP_REQUEST */

	vTaskDelay(pdMS_TO_TICKS(3000));

#if HTTP_USE_HTTPS_REQUEST
	ESP_LOGI(MODULE_NAME, "/************************ TESTING HTTPS REQUEST************************/\r\n");
	https_client_req();
	ESP_LOGI(MODULE_NAME, "/************************ END TESTING HTTPS REQUEST************************/\r\n");

#endif /* End of HTTP_USE_HTTPS_REQUEST */

	while(1)
	{
		vTaskDelay(pdMS_TO_TICKS(500)); /* Delay to prevent WDT trigger */
	};
}

void http_client_req()
{
	static char http_resp_buffer[HTTP_RESP_MAX_LEN] = {0};
	int response_len = 0;
	/* HTTP client init */
    ESP_LOGI(MODULE_NAME, "Initializing HTTP client \r\n");
	esp_http_client_config_t config = {
	   .url = "http://api.ipify.org/",
	   .event_handler = http_event_handle,
	   .user_data = http_resp_buffer,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	assert(NULL != client);

	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK)
	{
		response_len = esp_http_client_get_content_length(client);
		ESP_LOGI(MODULE_NAME, "Status = %d, content_length = %d",
				   esp_http_client_get_status_code(client), response_len);
		//ESP_LOG_BUFFER_HEX(MODULE_NAME, http_resp_buffer, response_len);
		ESP_LOGI(MODULE_NAME, "HTTP response data: %.*s \r\n", response_len, http_resp_buffer);
	}
	else
	{
		ESP_LOGE(MODULE_NAME, "HTTP GET request failed: %s", esp_err_to_name(err));
	}
	esp_http_client_cleanup(client);
}

/* Send HTTPS request using dynamic allocation & chunking to stored large data*/
void https_client_req(void)
{
	large_payload_t recv_payload = {
			.p_payload = NULL,
			.payload_len = 0,
	};
	recv_payload.p_payload = malloc(HTTPS_DEFAULT_RECV_LEN);
	assert(NULL != recv_payload.p_payload); /* Check allocation data */

	esp_http_client_config_t https_request =
	{
		.event_handler = https_event_handle,
		.user_data = (void*)&recv_payload,
#if HTTPS_USE_ESP_CERT_BUNDLE
		.url = "https://www.howsmyssl.com/a/check",
		.crt_bundle_attach = esp_crt_bundle_attach,
#else /* HTTPS_USE_ESP_CERT_BUNDLE */
		.url =  "https://worldtimeapi.org/api/timezone/Europe/London/",
		.cert_pem = (char*)wordtime_cert,
#endif /* End of HTTPS_USE_ESP_CERT_BUNDLE */

	};

	esp_http_client_handle_t client = esp_http_client_init(&https_request);
	assert(NULL != client);

	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK)
	{
		ESP_LOGI(MODULE_NAME, "Status = %d, content_length = %d",
				   esp_http_client_get_status_code(client), recv_payload.payload_len);
		ESP_LOGI(MODULE_NAME, "HTTPS response data: %.*s \r\n", recv_payload.payload_len, recv_payload.p_payload);
	}
	else
	{
		ESP_LOGE(MODULE_NAME, "HTTPS GET request failed: %s", esp_err_to_name(err));
	}
	esp_http_client_cleanup(client);
}
