#ifndef __NET_TAP_H__
#define __NET_TAP_H__

#include "net_device.h"

class net_tap: public net_device
{
public:
    net_tap(const char *if_name);
    bool init(const char *if_name);
    int receive(uint8_t *buffer, int max_len);
    int send(uint8_t *buffer, int length);

protected:
    int m_fd;
};

#endif
