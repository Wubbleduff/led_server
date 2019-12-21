
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <assert.h>
#include <cstring>

#include <dlfcn.h>

typedef int SOCKET;

#define CODE_WOULD_BLOCK EAGAIN
#define CODE_IS_CONNECTED EISCONN
#define CODE_INVALID ECONNREFUSED


typedef void (*init_led_fn)(int num_leds, unsigned **strip_buffer);
typedef void (*render_to_led_fn)();
typedef void (*clear_led_fn)();
typedef void (*shutdown_led_fn)();

struct RendererData
{
  int width, height;
  unsigned *light_data;

  init_led_fn init_led;
  render_to_led_fn render_to_led;
  clear_led_fn clear_led;
  shutdown_led_fn shutdown_led;
};

static RendererData *renderer_data;

static const int NUM_LEDS_ON_STRIP = 256;
static const unsigned MAX_BRIGHTNESS_VALUE = 10;  



static void init_renderer()
{
  renderer_data = (RendererData *)malloc(sizeof(RendererData));
  renderer_data->width  = 16;
  renderer_data->height = 16;

  // Load led renderer dll
  void *dll_handle = dlopen("./led_renderer.dll", RTLD_LAZY);
  if(!dll_handle)
  {
    fprintf(stderr, "Could not load led renderer dll\n");
    assert(0);
  }

  renderer_data->init_led = (init_led_fn)dlsym(dll_handle, "init_led");
  renderer_data->render_to_led = (render_to_led_fn)dlsym(dll_handle, "render_to_led");
  renderer_data->clear_led = (clear_led_fn)dlsym(dll_handle, "clear_led");
  renderer_data->shutdown_led = (shutdown_led_fn)dlsym(dll_handle, "shutdown_led");



  renderer_data->init_led(NUM_LEDS_ON_STRIP, &(renderer_data->light_data));
}

static void render()
{
  renderer_data->render_to_led();
}

static void swap_frame()
{
  renderer_data->clear_led();
}

static void shutdown_renderer()
{
  renderer_data->shutdown_led();
  free(renderer_data);
}










static int get_last_error()
{
  return errno;
}

static SOCKET create_udp_socket()
{
  SOCKET socket_id = socket(AF_INET, SOCK_DGRAM, 0);
  if(socket_id == -1) fprintf(stderr, "Error opening socket_id: %i\n", get_last_error());

  return socket_id;
}

static void make_nonblocking(SOCKET socket_id)
{
  unsigned long mode = 1;
  int error = ioctl(socket_id, FIONBIO, &mode);
  if(error != 0)  fprintf(stderr, "Error opening socket: %i\n", get_last_error());
}

static void make_address(const char *ip_address_string, int port_number, sockaddr_in *out_address)
{
  // Create address
  out_address->sin_family = AF_INET;
  out_address->sin_port = htons((unsigned short)port_number);
  int error = inet_pton(AF_INET, ip_address_string, &(out_address->sin_addr.s_addr));
  if(error == -1) fprintf(stderr, "Error creating an address: %i\n", get_last_error());
}

static void make_address_for_any_ip(int port_number, sockaddr_in *out_address)
{
  // Create address
  out_address->sin_family = AF_INET;
  out_address->sin_port = htons((unsigned short)port_number);
  out_address->sin_addr.s_addr = INADDR_ANY;

  /*
  int error = inet_pton(AF_INET, address_string, &(address.sin_addr->s_addr));
  if(error == -1) fprintf(stderr, "Error creating an address: %i\n", error);
  */
}

static void bind_to_address(SOCKET socket_id, sockaddr_in *address)
{
  int error = bind(socket_id, (sockaddr *)address, sizeof(*address));
  if(error == -1) fprintf(stderr, "Error binding to given address. Error code: %i\n", get_last_error());
}

#if 0
static int connect_to_host(SOCKET socket_id, const char *address_string, int port)
{
  // Create address
  sockaddr_in address = {};
  address.sin_family = AF_INET;
  address.sin_port = htons((unsigned short)port);
  int error = inet_pton(AF_INET, address_string, &address.sin_addr.s_addr);
  if(error == -1) fprintf(stderr, "Error creating an address: %i\n", error);


  // Try to connect
  int code = connect(socket_id, (sockaddr *)(&address), sizeof(address));
  int status = get_last_error();

  // While not connected, try to reconnect
  while(code != 0 && status != CODE_IS_CONNECTED)
  {
    code = connect(socket_id, (sockaddr *)(&address), sizeof(address));
    status = get_last_error();

    if(status == CODE_INVALID) { error = CODE_INVALID; break; }
  }

  return error;
}

static int send_data(const void *data, unsigned bytes)
{
  // Send data over TCP
  long int bytes_queued = send(socket_id, (const char *)data, bytes, 0);
  if(bytes_queued == -1) fprintf(stderr, "Error sending data: %i\n", get_last_error());

  return static_cast<int>(bytes_queued); // Can safely cast because of passing in bytes as max bytes
}
#endif

static bool recieve_data(SOCKET socket_id, void *buffer, int bytes, int *bytes_read)
{
  // Listen for a response
  sockaddr from_address = {};
  socklen_t from_size = sizeof(from_address);
  long int bytes_read_from_socket = recvfrom(socket_id, (char *)buffer, bytes, 0, &from_address, &from_size);

  *bytes_read = static_cast<int>(bytes_read_from_socket); // Can safely cast because of passing in bytes as max bytes

  // Check if bytes were read
  if(*bytes_read == -1)
  {
    // Bytes weren't read, make sure there's an expected error code
    if(get_last_error() != CODE_WOULD_BLOCK)
    {
      fprintf(stderr, "Error recieving data: %i\n", get_last_error());
    }
    return false;
  }
  else
  {
    // Bytes were read
    return true;
  }
}

static void no_more_sending(SOCKET socket_id)
{
  int error = shutdown(socket_id, SHUT_WR);
  if(error != 0) fprintf(stderr, "Error shutting down: %i\n", get_last_error());
}

static void close_socket(SOCKET socket_id)
{
  // Close the socket
  int error = close(socket_id);
  if(error == -1) fprintf(stderr, "Error closing socket: %i\n", get_last_error());
}


int main(int argc, const char **argv)
{
  (void)argc;
  (void)argv;

  int port_number = 4242;
  const int IN_BUFFER_SIZE = 1024;


  init_renderer();


  SOCKET udp_socket = create_udp_socket();

  sockaddr_in address = {};
  make_address_for_any_ip(port_number, &address);

  bind_to_address(udp_socket, &address);

  char *in_buffer = (char *)malloc(IN_BUFFER_SIZE + 1);

  while(true)
  {
    int bytes_read;
    bool read_data = recieve_data(udp_socket, in_buffer, IN_BUFFER_SIZE, &bytes_read);
    //printf("Bytes read: %i\n", bytes_read);

    if(read_data)
    {
      /*
      if(bytes_read >= IN_BUFFER_SIZE)
      {
        fprintf(stderr, "Read in more bytes than the buffer can hold. Buffer size: %i - read bytes: %i\n", IN_BUFFER_SIZE - 1, bytes_read);
      }

      in_buffer[bytes_read] = '\0';
      printf("%s", in_buffer);
      fflush(stdout);
      */

      memcpy(renderer_data->light_data, in_buffer, bytes_read);


      /*
      float alpha = 1.0f;
      int r = MAX_BRIGHTNESS_VALUE * 0.0f * alpha;
      int g = MAX_BRIGHTNESS_VALUE * 0.0f * alpha; 
      int b = MAX_BRIGHTNESS_VALUE * 1.0f * alpha; 
      unsigned value = (b << 16) | (g << 8) | (r << 0);

      // if(row % 2 == 1) column = (renderer_data->width - 1) - column;

      renderer_data->light_data[0 * renderer_data->width + 2] = value;
      renderer_data->light_data[2 * renderer_data->width + 0] = value;
      */

      render();
      swap_frame();
      memset(in_buffer, 0, IN_BUFFER_SIZE);
    }
  }

  free(in_buffer);
  close_socket(udp_socket);
  shutdown_renderer();

  return 0;
}

