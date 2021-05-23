#ifndef __NET_DEVICE_H__
#define __NET_DEVICE_H__

#include <stdint.h>

class net_device
{
public:
    virtual int receive(uint8_t *buffer, int max_len) = 0;
    virtual int send(uint8_t *buffer, int length) = 0;
};

#endif
