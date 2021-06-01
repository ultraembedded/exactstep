#include "net_tap.h"

#ifdef INCLUDE_NET_DEVICE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

//------------------------------------------------------------
// Constructor
//------------------------------------------------------------
net_tap::net_tap(const char *if_name)
{
    m_fd = -1;
    init(if_name);
}
//------------------------------------------------------------
// init: Intialise TAP based ethernet driver
//------------------------------------------------------------
bool net_tap::init(const char *if_name)
{
    struct ifreq ifr;
    if ((m_fd = open("/dev/net/tun", O_RDWR | O_NONBLOCK)) < 0)
    {
        perror("Opening /dev/net/tun");
        return false;
    }

    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
    strncpy(ifr.ifr_name, if_name, IFNAMSIZ);

    if ((ioctl(m_fd, TUNSETIFF, (void *)&ifr)) < 0)
    {
        fprintf(stderr, "ERROR: Cannot open TAP device '%s' (permissions?)\n", if_name);
        close(m_fd);
        m_fd = -1;
        return false;
    }

    return true;
}
//------------------------------------------------------------
// receive: Poll for receive data
//------------------------------------------------------------
int net_tap::receive(uint8_t *buffer, int max_len)
{
    if (m_fd < 0)
        return 0;

    int l = read(m_fd, (char*)buffer, max_len);
    if (l < 0 && errno == EAGAIN)
        return 0;
    
    if(l < 0)
    {
        perror("Reading from interface");
        close(m_fd);
        return 0;
    }

    return l;
}
//------------------------------------------------------------
// send: Send ethernet packet
//------------------------------------------------------------
int net_tap::send(uint8_t *buffer, int length)
{
    if (m_fd < 0)
        return 0;

    return write(m_fd, buffer, length) == length; 
}
#endif
