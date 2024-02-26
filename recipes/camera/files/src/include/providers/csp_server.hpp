#include "message_queue.hpp"
#include "vimba_provider.hpp"
#include "types.hpp"
#include <functional>
#include <string>

#ifndef CSPSERVER_H
#define CSPSERVER_H

void server(void);

void server_init(std::function<void(CaptureMessage params, VimbaProvider*, MessageQueue*, std::vector<VmbCPP::CameraPtr>)> callback,
				VimbaProvider* vmbProvider, MessageQueue* mq, std::vector<VmbCPP::CameraPtr> cameras);

#endif