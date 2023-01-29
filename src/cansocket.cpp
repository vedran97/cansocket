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
#include <boost/static_assert.hpp>
namespace cansocket
{
  /* CAN DLC to real data length conversion helpers */
  static const std::vector<unsigned char> dlc2len = {0, 1, 2, 3, 4, 5, 6, 7,
            8, 12, 16, 20, 24, 32, 48, 64};

  /* get data length from raw data length code (DLC) */
  unsigned char can_fd_dlc2len(unsigned char dlc)
  {
    return dlc2len.at(dlc & 0x0F);
  }

  static const std::vector<unsigned char> len2dlc = {0, 1, 2, 3, 4, 5, 6, 7, 8,		/* 0 - 8 */
            9, 9, 9, 9,				/* 9 - 12 */
            10, 10, 10, 10,				/* 13 - 16 */
            11, 11, 11, 11,				/* 17 - 20 */
            12, 12, 12, 12,				/* 21 - 24 */
            13, 13, 13, 13, 13, 13, 13, 13,		/* 25 - 32 */
            14, 14, 14, 14, 14, 14, 14, 14,		/* 33 - 40 */
            14, 14, 14, 14, 14, 14, 14, 14,		/* 41 - 48 */
            15, 15, 15, 15, 15, 15, 15, 15,		/* 49 - 56 */
            15, 15, 15, 15, 15, 15, 15, 15};	/* 57 - 64 */

  /* map the sanitized data length to an appropriate data length code */
  unsigned char can_fd_len2dlc(unsigned char len)
  {
    if (len > 64)
      return 0xF;

    return len2dlc.at(len);
  }
  CanSocket::CanSocket(std::string canChannelName,
                       std::vector<unsigned int> &canIds,
                       eSocketType socketType = eSocketType::READ_WRITE)
      : socketType(socketType)
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
     * Setup Addr object before using interfaceReq
    */
    addr.can_family = AF_CAN;
    addr.can_ifindex = interfaceRequest.ifr_ifindex;
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
    for (const auto &id : canIds)
    {
      recieveFilters.push_back(can_filter{(id | CAN_ID_OFFSET), 0xFFFF});
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
    if (setsockopt(socket, SOL_CAN_RAW, CAN_RAW_LOOPBACK, &loopback, sizeof(loopback)) < 0)
    {
      throw std::runtime_error("Failed to set loopback settings");
    }
    int recv_own_msgs = 0;
    if (setsockopt(socket, SOL_CAN_RAW, CAN_RAW_RECV_OWN_MSGS,
                   &recv_own_msgs, sizeof(recv_own_msgs)) < 0)
    {
      throw std::runtime_error("Failed to set loopback settings");
    }
    /**
     * Set socket to receive raw CAN_FD frames
     */
    int canFdOn = 1;
    if(setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FD_FRAMES, &canFdOn, sizeof(canFdOn))<0)
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

    if (bind(socket, (struct sockaddr *)(&addr), sizeof(addr)) < 0){
      perror("Bind Failed");
      throw std::runtime_error("Could not bind socket");
    }
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

  [[nodiscard]] CanFDMsg CanSocket::read() const 
  {
    if(socketFD<0){
      throw std::runtime_error("Socket in invalid state");
    }
    struct canfd_frame cfd;
    auto nbytes = ::read(socketFD, &cfd, CANFD_MTU);
    if (nbytes < 0)
    {
      perror("CAN raw socket read failed");
      throw std::runtime_error("Socket Read failed");
    }
    if ((size_t)nbytes < CANFD_MTU)
    {
      throw std::runtime_error("read incomplete frame");
    }
    return {cfd.can_id,{std::begin(cfd.data), std::end(cfd.data)}};
  }

  void CanSocket::write(CanFDMsg message,bool brs) const
  {
    if(socketFD<0){
      throw std::runtime_error("Socket in invalid state");
    }

    struct canfd_frame cfdFrame;
    cfdFrame.can_id = message.id;
    std::copy(message.data.begin(),message.data.end(),std::begin(cfdFrame.data));
    cfdFrame.len = can_fd_dlc2len(can_fd_len2dlc(message.data.size()));
    cfdFrame.flags = brs?CANFD_BRS:0;

    if (::write(socketFD, &cfdFrame, CANFD_MTU) != CANFD_MTU) {
      throw std::runtime_error("Socket write failed");
    }
  }

} // namespace cansocket
