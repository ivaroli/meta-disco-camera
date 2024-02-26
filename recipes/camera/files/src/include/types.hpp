#ifndef TYPES_H
#define TYPES_H

#include <stdlib.h>
#include <stdint.h>
#include <iostream>

typedef struct ImageBatch {
    unsigned int height;
    unsigned int width;
    unsigned int channels;
    unsigned int num_images;
    unsigned int data_size;
    int shm_key;
    unsigned char *data;
} ImageBatch;

typedef struct ImageBatchMessage {
    long mtype;
    unsigned int height;
    unsigned int width;
    unsigned int channels;
    unsigned int num_images;
    unsigned int data_size;
    int shm_key;
} ImageBatchMessage;

typedef struct CaptureMessage {
    int NumberOfImages;
    int Exposure;
    int ISO;
    std::string Camera;
} CaptureMessage;

#endif