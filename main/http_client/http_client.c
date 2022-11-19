#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

#include "http_client.h"
#include <esp_http_client.h>

#define MODULE_NAME					"HTTP_CLIENT:"
#define HTTP_RESP_MAX_LEN			1048
#define HTTP_USE_DYN_ALLOC			0

static char http_resp_buffer[HTTP_RESP_MAX_LEN] = {0};

esp_err_t _http_event_handle(esp_http_client_event_t *evt)
{
    switch(evt->event_id) {
        case HTTP_EVENT_ERROR:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ERROR");
            break;
        case HTTP_EVENT_ON_CONNECTED:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ON_CONNECTED");
        	ESP_LOGI(MODULE_NAME, "Client handler to: 0x%X \r\n", (uint32_t)evt->client);

            break;
        case HTTP_EVENT_HEADER_SENT:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_HEADER_SENT");
            break;
        case HTTP_EVENT_ON_HEADER:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ON_HEADER");
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        case HTTP_EVENT_ON_DATA:
            ESP_LOGI(MODULE_NAME, "HTTP_EVENT_ON_DATA, len=%d \r\n", evt->data_len);

            /* TMT: Get data from event into response buffer */
            memcpy(http_resp_buffer, evt->data, evt->data_len);
			ESP_LOGI(MODULE_NAME, "HTTP response data: %.*s \r\n",evt->data_len, (char*)http_resp_buffer);

			#if 0
			if (!esp_http_client_is_chunked_response(evt->client))
            {
                printf("[Response data]: %.*s \r\n", evt->data_len, (char*)evt->data);
            }
			#endif /* End of 0 */


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

void http_client_req()
{
	#if HTTP_USE_DYN_ALLOC
	char* http_resp_buffer = (char*)calloc(1, HTTP_RESP_MAX_LEN);
	assert(NULL != http_resp_buffer);
	#endif /* End of HTTP_USE_DYN_ALLOC */
	int response_len = 0;


	/* HTTP client init */
    ESP_LOGI(MODULE_NAME, "Initializing HTTP client \r\n");
	esp_http_client_config_t config = {
	   .url = "http://api.ipify.org/",
	   .event_handler = _http_event_handle,
	   .user_data = http_resp_buffer,
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	assert(NULL != client);
	ESP_LOGI(MODULE_NAME, "Update HTTP client handler to: 0x%X \r\n", (uint32_t)client);

	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK)
	{
		response_len = esp_http_client_get_content_length(client);
		ESP_LOGI(MODULE_NAME, "Status = %d, content_length = %d",
				   esp_http_client_get_status_code(client), response_len);
		ESP_LOG_BUFFER_HEX(MODULE_NAME, http_resp_buffer, response_len);
		ESP_LOGI(MODULE_NAME, "HTTP response data: %.*s \r\n", response_len, http_resp_buffer);
	}
	else
	{
		ESP_LOGE(MODULE_NAME, "HTTP GET request failed: %s", esp_err_to_name(err));
	}
	esp_http_client_cleanup(client);
}
