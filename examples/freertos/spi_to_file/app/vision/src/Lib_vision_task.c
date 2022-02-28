

#include <platform.h>
#include "rtos/drivers/intertile/api/rtos_intertile.h"

#if ON_TILE(0)


#include "Lib_vision_task.h"
#include "Vision_api.h"

#define IMAGE_SIZE (96 * 96) //This is also defined in person_detect_task, and must be equivalent.

typedef struct Lib_vision_args_t{
        QueueHandle_t input_queueA;
        QueueHandle_t input_queueB;
}lib_vision_args_t;

void lib_vision_task(void *args){
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q = targs->input_queueB;
  uint8_t *img_buf = NULL;
  uint8_t final_img_buf[IMAGE_SIZE];

  //Image writing variables:
  int width = 99;
  int height = 99;
  int channels = 1;
  char Filepath[] = "./images/output_image.bmp";

  while(1){

    xQueueReceive(q, &img_buf, portMAX_DELAY);
    /* img_buf[i%2] contains the values we want to print over the api */
    for (int i = 0; i < (IMAGE_SIZE * 2); i++) {
      if ((i % 2)) {
        final_img_buf[i >> 1] = img_buf[i];
      }
    }
    vPortFree(img_buf);//*/

    //rtos_printf("\nVision Printing Image");
    ArrayToFile(final_img_buf,width,height,channels,Filepath);
    //rtos_printf("\nVision Moving to next image");
  }
}

///*
static void lib_vision_runner_rx(void *args) {
  
  lib_vision_args_t *targs = (lib_vision_args_t *)args;
  QueueHandle_t q_A = targs->input_queueA;
  QueueHandle_t q_B = targs->input_queueB;
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

    for(int i = 0; i < 10; i++){
      rtos_printf("\nBuffer Value: %d", input_tensor[i]);
    }

    //
    rtos_printf("\nV_rx sending input tensor..."); 
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