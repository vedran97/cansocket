#include "cansocket/cansocket.hpp"

#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <stdexcept>
#include <unistd.h>
#include <errno.h>

#include <cstring>
#include <algorithm>
#include <iostream>
namespace cansocket
{
  CanSocket::CanSocket(std::string canChannelName,
                       std::vector<unsigned int> &canIds,
                       eSocketType socketType = eSocketType::READ_WRITE)
  {
    socketFD = -1;
    int mtu;
    /**
     * Create a RAW can socket
     */
    auto socket = ::socket(PF_CAN, SOCK_RAW, CAN_RAW);
    if (socket < 0)
      throw std::runtime_error("Could not create socket");

    auto interfaceRequest = ifreq();
    /**
     * Set interface name : to received name, for eg it can be like "can0/vcan0"
     */
    strncpy(interfaceRequest.ifr_name, canChannelName.c_str(), IFNAMSIZ - 1);
    interfaceRequest.ifr_name[IFNAMSIZ - 1] = '\0'; // Guarantees that string name is null terminated
    interfaceRequest.ifr_ifindex = if_nametoindex(interfaceRequest.ifr_name);
    if (!interfaceRequest.ifr_ifindex)
    {
      throw std::runtime_error("Failed to get interface index from it's name");
    }
    /**
     * Failed to get MTU of can interface
     */
    if (ioctl(socket, SIOCGIFMTU, &interfaceRequest) < 0)
    {
      throw std::runtime_error("SIOCGIFMTU");
    }
    /**
     * Check if registered CAN interface is CAN FD capable or not
     */
    mtu = interfaceRequest.ifr_mtu;
    if (mtu != CANFD_MTU)
    {
      throw std::runtime_error("CAN interface is not CAN FD capable - sorry.\n");
    }
    /**
     * Make receive filters -> this step is useless if this socket is WRITE_ONLY
     */
    auto recieveFilters = std::vector<can_filter>();
    for(const auto& id:canIds){
      recieveFilters.push_back(can_filter{(id|CAN_ID_OFFSET),0xFFFF});
    }
    /**
     * Set RAW filters on the socket
     */
    if (
        setsockopt(socket,
                   SOL_CAN_RAW,
                   CAN_RAW_FILTER,
                   recieveFilters.data(),
                   sizeof(can_filter) * recieveFilters.size()) < 0)
    {
      throw std::runtime_error("Failed to set filters");
    }
    /**
     * Set loopback settings
     * loopback is set to 1 so that candump <interface_name> can read the messsages sent on this socket
     * recv_own_messages is set to 0 -> Messages sent over this socket aren't looped
     * back and read again in socket's read queue
     */
    int loopback = 1;
    if(setsockopt(socket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback))<0){
      throw std::runtime_error("Failed to set loopback settings");
    }
    int recv_own_msgs = 0;
    if(setsockopt(socket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
               &recv_own_msgs, sizeof(recv_own_msgs))<0){
      throw std::runtime_error("Failed to set loopback settings");
    }
    /**
     * Set socket to receive raw CAN_FD frames
     */
    int canFdOn = 1;
    setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canFdOn, sizeof(canFdOn));
    if (canFdOn != 0)
      throw std::runtime_error("Could could not switch to Can FD Mode");
    /**
     * Set socket's timeout to 1 millisecond
     */
    struct timeval tv = {0, 1000};
    if (setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv) < 0)
    {
      perror("Error setting CAN timeout");
      throw std::runtime_error("Error setting CAN timeout");
    }
    /**
     * Bind socket to address
     */
    addr.can_family = AF_CAN;
    addr.can_ifindex = interfaceRequest.ifr_ifindex;
    if (bind(socket, (struct sockaddr *)(&addr), sizeof(addr)) < 0)
      throw std::runtime_error("Could not bind socket");
    /**
     *  Shutdown read or write funtionality if necessary
     */
    if (socketType != eSocketType::READ_WRITE)
    {
      if (shutdown(socket, static_cast<std::underlying_type<eSocketType>::type>(socketType)) < 0)
      {
        throw std::runtime_error("Could not assign operation mode to socket");
      }
    }
    socketFD = socket;
    return;
  }

  CanSocket::~CanSocket()
  {
    std::cout << " Closing socket" << std::endl;
    if (this->socketFD > 0)
    {
      ::close(socketFD);
    }
  }

} // namespace cansocket
