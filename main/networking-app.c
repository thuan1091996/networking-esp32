/*************** INTERFACE CHANGE LIST **************************************
*
*    Date       	Software Version    Initials        Description
*  27 Nov 2022         0.0.1      		Itachi      Interface Created.
*
*****************************************************************************/
/* @file:	networking-app.c
 * @brief: 	networking application with ESP32.c
 */

/******************************************************************************
* Includes
*******************************************************************************/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event_base.h"

#include "task_common/task_common.h"
#include "http_client/http_client.h"
#include "socket_client/tcp_client_socket.h"


/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#if !1
#define EXAMPLE_ESP_WIFI_SSID      "Sesame"
#define EXAMPLE_ESP_WIFI_PASS      "sesame@262"
#elif 0
#define EXAMPLE_ESP_WIFI_SSID      "Trung Nguyen-2.4G"
#define EXAMPLE_ESP_WIFI_PASS      "trungnguyen"
#else /* MANUALLY_CHOOSE_SSID */
#define EXAMPLE_ESP_WIFI_SSID      "Moda House Coffee"
#define EXAMPLE_ESP_WIFI_PASS      "0938346538"
#endif /* End of MANUALLY_CHOOSE_SSID */

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT 			BIT0
#define WIFI_FAIL_BIT      			BIT1


/******************************************************************************
* User definitions
*******************************************************************************/
#define MODULE_NAME					"MAIN_APP "
#define EXAMPLE_ESP_MAXIMUM_RETRY  	CONFIG_ESP_MAXIMUM_RETRY

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
TaskInitParams_t const TasksTable[] =
{
 // Function pointer,	String Name,	Stack size,		Parameter,	Priority,	Task Handle
   {&http_task,	"HTTP Task",	HTTP_TASK_STACK_SIZE,  NULL, HTTP_TASK_PRIORITY, &xHTTP_handler},
};



/******************************************************************************
* Function Definitions
*******************************************************************************/
void wifi_event_handler(void* event_handler_arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
	static int s_retry_num = 0;
	ESP_LOGI(MODULE_NAME, "Event handler invoked \r\n");
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
        ESP_LOGI(MODULE_NAME, "WIFI_EVENT_STA_START");
		ESP_LOGI(MODULE_NAME, "Wi-Fi STATION started successfully \r\n");
		esp_wifi_connect();
	}
	else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
        ESP_LOGI(MODULE_NAME, "WIFI_EVENT_STA_DISCONNECTED");
		if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
		{
			esp_wifi_connect();
			s_retry_num++;
			ESP_LOGI(MODULE_NAME, "retry to connect to the AP");
		}
		else
		{
			xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
		}
		ESP_LOGI(MODULE_NAME,"connect to the AP fail");
	}
	else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ESP_LOGI(MODULE_NAME, "IP_EVENT_STA_GOT_IP");
		ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
		ESP_LOGI(MODULE_NAME, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
		s_retry_num = 0;
		xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
	}
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    /* Create and init lwIP related stuffs */
    ESP_ERROR_CHECK(esp_netif_init());

    /* Create default event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Create default network interface instance binding to netif */
    esp_netif_create_default_wifi_sta();

    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(MODULE_NAME, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
        ESP_LOGI(MODULE_NAME, "connected to ap SSID:%s password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    	http_client_init();
    	tcp_client_socket_init();
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        ESP_LOGI(MODULE_NAME, "Failed to connect to SSID:%s, password:%s", EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    }
    else
    {
        ESP_LOGE(MODULE_NAME, "UNEXPECTED EVENT");
    }
}

void app_main(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ESP_OK != ret)
    {
    	ESP_ERROR_CHECK(nvs_flash_erase());
    	ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_LOGI(MODULE_NAME, "NVS Flash initialized \r\n");

    ESP_LOGI(MODULE_NAME, "Initializing Wi-Fi station \r\n");
    wifi_init_sta();
}
