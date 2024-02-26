#include <memory>
#ifndef CSPSERVERPOSIX_H
#define CSPSERVERPOSIX_H

static void * task_router(void * param);

static void * task_server(void * param);

int router_start(void);

int server_start(void);

#endif