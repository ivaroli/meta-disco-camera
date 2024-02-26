#include "csp_server.hpp"
#include "types.hpp"
#include "csp_parser.hpp"
#include <csp/csp_debug.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <functional>
#include <pthread.h>
#include <csp/csp.h>
#include <csp/drivers/usart.h>
#include <csp/drivers/can_socketcan.h>
#include <csp/interfaces/csp_if_zmqhub.h>
#include "message_queue.hpp"
#include "vimba_provider.hpp"
#include <memory>


/* These three functions must be provided in arch specific way */
int router_start(void);
int server_start(void);

#define SERVER_PORT	10
#define ZMQ_ADDRESS "localhost"
#define SERVER_ADDRESS 2

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t condition = PTHREAD_COND_INITIALIZER;
std::shared_ptr<char> shared_packet;

void server(void) {
	csp_print("Server task started\n");

	csp_socket_t sock = {0};
	csp_bind(&sock, CSP_ANY);
	csp_listen(&sock, 10);

	while (1) {
		csp_conn_t *conn;
		if ((conn = csp_accept(&sock, 10000)) == NULL) {
			/* timeout */
			continue;
		}

		csp_packet_t *packet;
		while ((packet = csp_read(conn, 50)) != NULL) {
			switch (csp_conn_dport(conn)) {
			case SERVER_PORT:
				pthread_mutex_lock(&mutex);
				
				shared_packet = std::shared_ptr<char>(new char[packet->length], std::default_delete<char[]>()); //std::make_shared<char>(packet->length);
				memcpy(shared_packet.get(), packet->data, packet->length*sizeof(uint8_t));
				
				pthread_cond_signal(&condition);
				pthread_mutex_unlock(&mutex);
				
				csp_buffer_free(packet); 
				break;

			default:
				/* Call the default CSP service handler, handle pings, buffer use, etc. */
				csp_service_handler(packet);
				break;
			}
		}

		/* Close current connection */
		csp_close(conn);

	}

	return;

}

/* Initialization of CSP */
void server_init(std::function<void(CaptureMessage params, VimbaProvider*, MessageQueue*, std::vector<VmbCPP::CameraPtr>)> callback,
				VimbaProvider* vmbProvider, MessageQueue* mq, std::vector<VmbCPP::CameraPtr> cameras) {
    csp_print("Initialising CSP\n");
	csp_init();
	router_start();
	csp_iface_t * default_iface = NULL;

    csp_print("Connection table\r\n");
    csp_conn_print_table();

    csp_print("Interfaces\r\n");
    csp_iflist_print();

    int error = csp_zmqhub_init(SERVER_ADDRESS, ZMQ_ADDRESS, 0, &default_iface);
    if (error != CSP_ERR_NONE) {
        csp_print("failed to add ZMQ interface [%s], error: %d\n", ZMQ_ADDRESS, error);
        exit(1);
    }
    default_iface->is_default = 1;

    server_start();

	while (true) {
		pthread_mutex_lock(&mutex);
		pthread_cond_wait(&condition, &mutex);

		csp_print("Packet received on SERVER_PORT: %s\n", (char *) shared_packet.get());
		if(shared_packet.get() != nullptr){
			CaptureMessage msg;
			std::string input(shared_packet.get());
			parseMessage(input, msg);
			shared_packet.reset();

			callback(msg, vmbProvider, mq, cameras); // callback to capture images
		}
		pthread_mutex_unlock(&mutex);
    }
}
