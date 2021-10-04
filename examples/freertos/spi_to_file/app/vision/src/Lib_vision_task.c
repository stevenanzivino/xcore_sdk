

#include <platform.h>
#include "rtos/drivers/intertile/api/rtos_intertile.h"

#if ON_TILE(0)

#include "Lib_vision_task.h"
#include "Vision_api.h"

#define IMAGE_SIZE (96 * 96) //This is also defined in person_detect_task, and must be equal to it.

typedef struct Lib_vision_args_t{
        QueueHandle_t input_queue;
        rtos_intertile_address_t *intertile_addr;
        //rtos_gpio_t gpio_ctx;
}lib_vision_args_t;

//This file is a lib_vision copy of vision_api

void lib_vision_task(void *args){
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t input_queue = targs->input_queue;
  uint8_t *img_buf = NULL;
  uint8_t final_img_buf[IMAGE_SIZE];
/*
In a loop, wait for communciations.
Format communcation into an array.
Define all constants into correct format for the api call
make call
repeat loop
*/
  //char Filepath[] = {'o','u','t','p','u','t','_','i','m','a','g','e','.','b','m','p'};
  /*
  rtos_printf("\nInitialized Lib_vision Variables");*/
  rtos_printf("\n Reached LibVision Thread Loop");
  while(1){
    //rtos_printf("\n Vision waiting for image...");
    //Get Communications here, uh somehow.
    //rtos_printf("Wait for next image...\n");
    //xQueueReceive(input_queue, &img_buf, portMAX_DELAY);                  //Recieves image ptr*/

    /* img_buf[i%2] contains the values we want to print over the api */
    /*for (int i = 0; i < (IMAGE_SIZE * 2); i++) {
      if ((i % 2)) {
        final_img_buf[i >> 1] = img_buf[i];
      }
    }
    vPortFree(img_buf);*/




    //Vision_API_Void();

    //rtos_printf("\n Vision recieved image.");
  }
  
  rtos_printf("\nCalling Get Image Pointer Here:");
  //writeImage(ai_img_buf,96,96,1, Filepath);
  rtos_printf("\nEnd Call to Get Image Pointer:");
  rtos_printf("\n");

}

///*
static void lib_vision_runner_rx(void *args) {
  
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q = targs->input_queue;
  rtos_intertile_address_t *adr = targs->intertile_addr;
  uint8_t *input_tensor;
  int input_tensor_len;
//*/
  rtos_printf("\nLib_vision_runner_rx <While>");
  while (1) {
    //input_tensor_len = rtos_intertile_rx(adr->intertile_ctx, adr->port,
    //                                     (void **)&input_tensor, portMAX_DELAY);
    //xQueueSend(q, &input_tensor, portMAX_DELAY);
  }
  
}
//*/
void lib_vision_task_create(rtos_intertile_address_t *intertile_addr,
    unsigned priority,QueueHandle_t input_queue){

  rtos_printf("\nlib_vision_task_create reached");
  lib_vision_args_t *args = pvPortMalloc(sizeof(lib_vision_args_t));

  //configASSERT(args);

  args->intertile_addr = intertile_addr;
  args->input_queue = input_queue;

  xTaskCreate((TaskFunction_t)lib_vision_task, "lib_vision_task",
          RTOS_THREAD_STACK_SIZE(lib_vision_task), args, priority,
          NULL);

  xTaskCreate((TaskFunction_t)lib_vision_runner_rx, "lib_vision_runner_rx",
          RTOS_THREAD_STACK_SIZE(lib_vision_runner_rx), args, priority,
          NULL);
  
}

#endif //On_Tile(0)