#ifndef MESSAGEQUEUE_H
#define MESSAGEQUEUE_H

#include <stdlib.h>
#include "types.hpp"

class MessageQueue{
    private:
        int createMemorySpace(size_t size);
        void* insertMemory(unsigned char *data, size_t size, int shm_id);
        bool sendMessage(ImageBatchMessage batch);

    public:
        MessageQueue();
        ~MessageQueue();
        bool SendImage(ImageBatch batch);
};

#endif