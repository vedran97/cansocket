#ifndef CANSOCKET__CANSOCKET_HPP_
#define CANSOCKET__CANSOCKET_HPP_

#include "cansocket/visibility_control.h"
#include <linux/can.h>
#include <linux/can/raw.h>
#include <vector>
#include <string>
namespace cansocket
{
  /**
   * This is a CAN socket wrapper
   * Targets : Not copyable , Defined destructor
   * Can be constructed in place in a std::vector/std::array
   * Solution : Implement rule of 5
   * Use vector's emplace_back perfect forwarding and reserve functions to
   * avoid copy and moves
   */
  class CanSocket
  {
  private:
    /**
     * This is an example offset, the CAN_SOCKET mask
     * depends on the user's chosen CAN_ID design
     * The Can Socket is configured to receive only those messages which match this condition : canID|CAN_ID_OFFSET
     * For example, if the IDs for this socket are 0x01 0x02 ;
     * then this socket will receive can messages with 0x06 and 0x07
     */
    static const constexpr unsigned int CAN_ID_OFFSET = 0x05;
    int socketFD;
    struct sockaddr_can addr;
    /**
     * Delete Value semantics
     */
    CanSocket &operator=(const CanSocket &) = delete;
    CanSocket(const CanSocket &) = delete;

  public:
    enum class eSocketType : unsigned char
    {
      WRITE_ONLY,
      READ_ONLY,
      READ_WRITE
    };
    explicit CanSocket(std::string canChannelName, std::vector<unsigned int> &canIds, eSocketType socketType);
    ~CanSocket();
    /**
     *  Need to maintain a move policy
     *  a Move from the socket keeps it in an invalid state
     */
    CanSocket(CanSocket &&movedFrom) noexcept
    {
      this->socketFD = movedFrom.socketFD;
      movedFrom.socketFD = -1;
    }
    CanSocket &operator=(CanSocket &&movedFrom) noexcept
    {
      this->socketFD = movedFrom.socketFD;
      movedFrom.socketFD = -1;
      return *this;
    }
  };

} // namespace cansocket

#endif // CANSOCKET__CANSOCKET_HPP_
