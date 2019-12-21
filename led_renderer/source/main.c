

#include "renderer.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <math.h>
#include <time.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/input.h>


static unsigned *frame_buffer;
static int led_count = 1024;
static int running;


void init_input()
{
}

void read_input()
{
  
}


void update_and_draw(float dt)
{
  (void)dt;
  int i;
#if 0
  static float shuttle_pos = 0.0f;
  float shuttle_width = 200.0f;

  static float counter;
  counter += dt * 0.002f;
  shuttle_pos = cos(counter) * 100.0f + 150.0f;

  float intensity = 0.0f;

  int begin = (int)(shuttle_pos - shuttle_width / 2.0f);
  int end = (int)(shuttle_pos + shuttle_width / 2.0f);
  float pixelPos = begin + 0.5;
  int i;
  for(i = begin; i < end; i++)
  {
    float diff = shuttle_pos - pixelPos;
    diff = (diff < 0.0f) ? -diff : diff;

    float intensity = 1.0f - (diff / (shuttle_width / 2.0f));
    intensity *= intensity * intensity;
    if(intensity > 1.0f) intensity = 1.0f;
    if(intensity < 0.0f) intensity = 0.0f;

    int pixel = i;
    if(pixel < 0) pixel = led_count + pixel;
    
    int color = (int)(96 * intensity) << 16;
    color += (int)(96 * intensity) << 0;
    
    frame_buffer[pixel % led_count] = color;

    pixelPos += 1.0f;
  }
#endif

#if 0
  static int pixel = 0;
  int red = 0;
  int green = 0;
  int blue = 10;
  frame_buffer[pixel] = (blue << 16) | (green << 8) | (red << 0);
  pixel += 1;
  pixel = pixel % led_count;
#endif

#if 1
  for(i = 0; i < led_count; i++)
  {
    int red = 255 / 4;
    int green = 255 / 4;
    int blue = 255 / 4;
    frame_buffer[i] = (blue << 16) | (green << 8) | (red << 0);
  }
#endif
  
}




static void ctrl_c_handler(int signum)
{
  (void)(signum);
  running = 0;
}

static void setup_handlers(void)
{
  struct sigaction sa =
  {
    .sa_handler = ctrl_c_handler,
  };

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}



#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)

int main(int argc, char *argv[])
{
  (void)argc;
  (void)argv;

  // Setting up OS interupt handlers
  setup_handlers();

  //led_count = 512;
  init_led(led_count, &frame_buffer);

  clock_t old = clock();






#define USE_INPUTx

#ifdef USE_INPUT
  int FileDevice;
  int ReadDevice;
  unsigned Index;
  struct input_event InputEvent[64];
  int version;
  unsigned short id[4];
  unsigned long bit[EV_MAX][NBITS(KEY_MAX)];
  
  //----- OPEN THE INPUT DEVICE -----
  if((FileDevice = open("/dev/input/event2", O_RDONLY)) < 0)    //<<<<SET THE INPUT DEVICE PATH HERE
  {
    perror("KeyboardMonitor can't open input device");
    close(FileDevice);
    return 1;
  }
  
  //----- GET DEVICE VERSION -----
  if(ioctl(FileDevice, EVIOCGVERSION, &version))
  {
    printf("KeyboardMonitor can't get version\n");
    close(FileDevice);
    return 1;
  }
  printf("Input driver version is %d.%d.%d\n", version >> 16, (version >> 8) & 0xff, version & 0xff);

  //----- GET DEVICE INFO -----
  ioctl(FileDevice, EVIOCGID, id);
  printf("Input device ID: bus 0x%x vendor 0x%x product 0x%x version 0x%x\n", id[ID_BUS], id[ID_VENDOR], id[ID_PRODUCT], id[ID_VERSION]);
  
  memset(bit, 0, sizeof(bit));
  ioctl(FileDevice, EVIOCGBIT(0, EV_MAX), bit[0]);
#endif




  running = 1;
  while(running)
  {
    clock_t now = clock();
    double delta = now - old; 
    old = now;

    // Go from microseconds to milliseconds
    float dt = (float)(delta * 0.001);
    //printf("dt (ms): %f\n", dt);






#ifdef USE_INPUT
    //read_input();
    ReadDevice = read(FileDevice, InputEvent, sizeof(struct input_event) * 64);
  
    if(ReadDevice < (int)sizeof(struct input_event))
    {
      //This should never happen
      printf("KeyboardMonitor error reading - keyboard lost?\n");
      close(FileDevice);
      return 1;
    }
    else
    {
      for(Index = 0; Index < ReadDevice / sizeof(struct input_event); Index++)
      {
        //We have:
        //InputEvent[Index].time  timeval: 16 bytes (8 bytes for seconds, 8 bytes for microseconds)
        //InputEvent[Index].type  See input-event-codes.h
        //InputEvent[Index].code  See input-event-codes.h
        //  InputEvent[Index].value    01 for keypress, 00 for release, 02 for autorepeat
            
        if(InputEvent[Index].type == EV_KEY)
        {
          if(InputEvent[Index].value == 2)
          {
            //This is an auto repeat of a held down key
            //std::cout << (int)(InputEvent[Index].code) << " Auto Repeat";
            //std::cout << std::endl;
          }
          else if(InputEvent[Index].value == 1)
          {
            //----- KEY DOWN -----
          //std::cout << (int)(InputEvent[Index].code) << " Key Down";    //input-event-codes.h
          //std::cout << std::endl;
          printf("Key down: %i\n", (int)(InputEvent[Index].code));
          }
          else if(InputEvent[Index].value == 0)
          {
            //----- KEY UP -----
            //std::cout << (int)(InputEvent[Index].code) << " Key Up";    //input-event-codes.h
            //std::cout << std::endl;
            printf("Key up: %i\n", (int)(InputEvent[Index].code));
          }
        }
      }
    }
#endif










    clear_led();

    update_and_draw(dt);

    render_to_led();
  }

  shutdown_led();

  printf("\n");
  return 0;
}


