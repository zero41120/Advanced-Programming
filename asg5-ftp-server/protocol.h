// $Id: protocol.h,v 1.3 2016-05-09 16:01:56-07 - - $

#ifndef __PROTOCOL__H__
#define __PROTOCOL__H__

#include <cstdint>
#include <cstring>
#include <iostream>
using namespace std;

#include "sockets.h"

/*
   ERROR, // an error flag to indicate an invalid header.
   EXIT,  // Internal to cix, not used in commnication.
   HELP,  // Internal to cix, not used in communication.
   GET,   // C->S Request a file from server. Payload = 0.
   LS,    // C->S Reequstion file information.
   PUT,   // C->S Payload = file bytes. File follow the header.
   RM,    // C->S Request to remove a file. Payload is 0.
   FILE,  // S->C Response to GET command.
   LSOUT, // S->C Response to LS command.
   ACK,   // S->C Response to PUT or RM for complete
   NAK,   // S->C Response to any command that fails.
*/
enum class cix_command : uint8_t {
   ERROR = 0, EXIT, GET, HELP, LS, PUT, RM, FILE, LSOUT, ACK, NAK,
};

size_t constexpr FILENAME_SIZE = 59;
struct cix_header {
   uint32_t nbytes {0};
   cix_command command {cix_command::ERROR};
   char filename[FILENAME_SIZE] {};
};

void send_packet (base_socket& socket,
                  const void* buffer, size_t bufsize);

void recv_packet (base_socket& socket, void* buffer, size_t bufsize);

ostream& operator<< (ostream& out, const cix_header& header);

string get_cix_server_host (const vector<string>& args, size_t index);

in_port_t get_cix_server_port (const vector<string>& args,
                               size_t index);

#endif

