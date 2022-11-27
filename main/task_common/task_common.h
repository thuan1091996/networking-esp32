/*************** INTERFACE CHANGE LIST **************************************    
*    
*    Date       	Software Version    Initials        Description    
*  27 Nov 2022         0.0.1           Itachi      Interface Created.    
*    
*****************************************************************************/    
/* @file:   task_common.h   
 * @brief:  This header contains   
 */ 
#ifndef MAIN_TASK_COMMON_TASK_COMMON_H_
#define MAIN_TASK_COMMON_TASK_COMMON_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include <string.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"

/******************************************************************************
* Preprocessor Constants
*******************************************************************************/


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
* Module Typedefs
*******************************************************************************/
typedef struct
{
    TaskFunction_t const TaskCodePtr;           /*< Pointer to the task function */
    const char * const TaskName;                /*< String task name             */
    const uint16_t StackDepth;                  /*< Stack depth                  */
    void * const ParametersPtr;                 /*< Parameter Pointer            */
    UBaseType_t TaskPriority;                   /*< Task Priority                */
    TaskHandle_t * const TaskHandle;            /*< Pointer to task handle       */
}TaskInitParams_t;


/******************************************************************************
* Variables
*******************************************************************************/
extern TaskHandle_t xHTTP_handler;
extern TaskInitParams_t const TasksTable[];

/******************************************************************************
* Function Prototypes
*******************************************************************************/


#endif /* MAIN_TASK_COMMON_TASK_COMMON_H_ */
