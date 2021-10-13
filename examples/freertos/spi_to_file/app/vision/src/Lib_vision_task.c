

#include <platform.h>
#include "rtos/drivers/intertile/api/rtos_intertile.h"

#if ON_TILE(0)


#include "Lib_vision_task.h"
#include "Vision_api.h"

#define IMAGE_SIZE (96 * 96) //This is also defined in person_detect_task, and must be equal to it.

typedef struct Lib_vision_args_t{
        QueueHandle_t input_queueA;
        QueueHandle_t input_queueB;
        //rtos_intertile_address_t *intertile_addr;
        //rtos_gpio_t gpio_ctx;
}lib_vision_args_t;

//This file is a lib_vision copy of vision_api

void lib_vision_task(void *args){
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q = targs->input_queueB;
  //rtos_intertile_address_t *adr = targs->intertile_addr;
  uint8_t *img_buf = NULL;
  uint8_t final_img_buf[IMAGE_SIZE];
  //int input_tensor_len;

  //Image writing variables:
  int width = 99;
  int height = 99;
  int channels = 1;
  //char Filepath[] = {'o','u','t','p','u','t','_','i','m','a','g','e','.','b','m','p'};
  char Filepath[] = "./images/output_image.bmp";
/*
In a loop, wait for communciations.
Format communcation into an array.
Define all constants into correct format for the api call
make call
repeat loop
*/
  /*
  rtos_printf("\nInitialized Lib_vision Variables");*/
  rtos_printf("\n Reached LibVision Thread Loop");
  while(1){

    rtos_printf("\nVision Wait for input tensor...");
    //rtos_printf("\nVision adr: intertile_ctx: %d, port: %d",adr->intertile_ctx,adr->port);
    xQueueReceive(q, &img_buf, portMAX_DELAY);
    rtos_printf("\nVision recieved input tensor");
    //rtos_printf("\nVision_task recieved: %d,%d,%d",img_buf[0],img_buf[1],img_buf[2]);
    /* img_buf[i%2] contains the values we want to print over the api */
    for (int i = 0; i < (IMAGE_SIZE * 2); i++) {
      if ((i % 2)) {
        final_img_buf[i >> 1] = img_buf[i];
      }
    }
    vPortFree(img_buf);//*/

    rtos_printf("\nVision Printing Image");
    ArrayToFile(img_buf,width,height,channels,Filepath);
    rtos_printf("\nVision Moving to next image");
  }
  
  rtos_printf("\nCalling Get Image Pointer Here:");
  //writeImage(ai_img_buf,96,96,1, Filepath);
  rtos_printf("\nEnd Call to Get Image Pointer:");
  rtos_printf("\n");

}

///*
static void lib_vision_runner_rx(void *args) {
  
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q_A = targs->input_queueA;
  QueueHandle_t q_B = targs->input_queueB;
  //rtos_intertile_address_t *adr = targs->intertile_addr;
  uint8_t *input_tensor;
  int input_tensor_len;
//*/
  rtos_printf("\nV_rx <While>");
  //rtos_printf("\nV_rx adr: intertile_ctx: %d, port: %d",adr->intertile_ctx,adr->port);
  while (1) {
    rtos_printf("\nV_rx Wait for input tensor...");
    xQueueReceive(q_A, &input_tensor, portMAX_DELAY);
    rtos_printf("\nV_rx recieved: %d,%d,%d",input_tensor[0],input_tensor[1],input_tensor[2]);
    //
    rtos_printf("\nV_rx sending input tensor..."); 
    //xQueueReceive(q_B, &input_tensor, portMAX_DELAY);
    if( xQueueSend( q_B, &input_tensor, pdMS_TO_TICKS( 1 ) ) == errQUEUE_FULL ){     //FreeRTOS memory management?
        debug_printf( "Camera frame lost\n" );
        vPortFree( input_tensor );
      }
    rtos_printf("\n V_rx, passed along tensor");
  }
  
}
//*/
void lib_vision_task_create(unsigned priority,QueueHandle_t input_queueA,QueueHandle_t input_queueB){

  rtos_printf("\nlib_vision_task_create reached");
  lib_vision_args_t *args = pvPortMalloc(sizeof(lib_vision_args_t));

  configASSERT(args);

  //args->intertile_addr = intertile_addr;
  args->input_queueA = input_queueA;
  args->input_queueB = input_queueB;

  rtos_printf("lib visition task stack is %u\n",RTOS_THREAD_STACK_SIZE(lib_vision_task));

  xTaskCreate((TaskFunction_t)lib_vision_task, "lib_vision_task",
          RTOS_THREAD_STACK_SIZE(lib_vision_task), args, priority,
          NULL);

  xTaskCreate((TaskFunction_t)lib_vision_runner_rx, "lib_vision_runner_rx",
          RTOS_THREAD_STACK_SIZE(lib_vision_runner_rx), args, priority,
          NULL);
  
}

#endif //On_Tile(0)