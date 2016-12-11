// $Id: cix.cpp,v 1.4 2016-05-09 16:01:56-07 - - $

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
using namespace std;

#include <libgen.h>
#include <sys/types.h>
#include <unistd.h>

#include "protocol.h"
#include "logstream.h"
#include "sockets.h"

logstream log (cout);
struct cix_exit: public exception {};

unordered_map<string,cix_command> command_map {
   {"exit", cix_command::EXIT},
   {"help", cix_command::HELP},
   {"ls"  , cix_command::LS  },
   {"get" , cix_command::GET },
   {"put" , cix_command::PUT },
   {"rm"  , cix_command::RM  },

};

void cix_help() {
   static const vector<string> help = {
      "exit         - Exit the program.  Equivalent to EOF.",
      "get filename - Copy remote file to local host.",
      "help         - Print help summary.",
      "ls           - List names of files on remote server.",
      "put filename - Copy local file to remote host.",
      "rm filename  - Remove file from remote server.",
   };
   for (const auto& line: help) cout << line << endl;
}

void cix_ls (client_socket& server) {
   cix_header header;
   header.command = cix_command::LS;
   log << "sending header " << header << endl;
   send_packet (server, &header, sizeof header);
   recv_packet (server, &header, sizeof header);
   log << "received header " << header << endl;
   if (header.command != cix_command::LSOUT) {
      log << "sent LS, server did not return LSOUT" << endl;
      log << "server returned " << header << endl;
   }else {
      char buffer[header.nbytes + 1];
      recv_packet (server, buffer, header.nbytes);
      log << "received " << header.nbytes << " bytes" << endl;
      buffer[header.nbytes] = '\0';
      cout << buffer;
   }
}

void cix_get (client_socket& server, const string& filename){
   // Condition check
   if (filename.size() >= FILENAME_SIZE){
     log << "GET Error: filenames longer than" 
     << FILENAME_SIZE << " characters are prohibited" << endl;
     return;
   }
   ifstream check (filename);
   if (check.good()){
      log << "GET Error: " << filename 
      << " exists in local machine." << endl;
     return;
   }

   // Create and set header
   cix_header header;
   header.nbytes = 0; 
   header.command = cix_command::GET; 
   snprintf(header.filename, FILENAME_SIZE, filename.c_str());

   // Send and recvie header.
   send_packet(server, &header, sizeof header);
   log << "HEADER OUT: " << header << endl;
   recv_packet(server, &header, sizeof header);
   log << "HEADER IN:  " << header << endl;

   // Check Header
   if (header.command != cix_command::FILE){
      log << "Command to server failed" << endl;
      return;
   }

   // File should be send immediately after a header.
   char buffer[header.nbytes + 1];
   log << "Download start" << endl;
   recv_packet(server, buffer, header.nbytes);
   log << "FILE IN:  " << header.nbytes << " bytes" << endl;
   buffer[header.nbytes] = '\0';
   ofstream output (filename.c_str());
   output.write(buffer, header.nbytes);
   output.close();
   log << "Download finish" << endl;
}

void cix_put (client_socket& server, const string& filename){
   // Condition check
   if (filename.size() >= FILENAME_SIZE){
     log << "GET Error: filenames longer than" 
     << FILENAME_SIZE << " characters are prohibited" << endl;
     return;
   }
   ifstream file_stream (filename.c_str());
   if (not file_stream.good()){
      log << "GET Error: " << filename << " no such file" << endl;
     return;
   }

   // Read file
   stringstream string_stream;
   string_stream << file_stream.rdbuf();
   string data = string_stream.str();

   // Set Header
   cix_header header;
   header.command = cix_command::PUT;
   header.nbytes = data.size();
   snprintf(header.filename, FILENAME_SIZE, filename.c_str());

   // Send file and header
   send_packet (server, &header, sizeof header);
   send_packet (server, data.c_str(), data.size()); 
   log << "HEADER OUT: " << header << endl;
   log << "DATA OUT  : " << data.size() << " bytes" << endl;

   // Check server status
   recv_packet (server, &header, sizeof header);
   log << "HEADER IN : " << header << endl;

   if (header.command != cix_command::ACK){
      log << "ERROR     : " << "Upload failed.";
   }
}

void cix_rm (client_socket& server, const string& filename){
   // Condition check
   if (filename.size() >= FILENAME_SIZE){
     log << "GET Error: filenames longer than" 
     << FILENAME_SIZE << " characters are prohibited" << endl;
     return;
   }

   // Set Header
   cix_header header;
   header.command = cix_command::RM;
   header.nbytes = 0;
   snprintf(header.filename, FILENAME_SIZE, filename.c_str());
   send_packet (server, &header, sizeof header);
   log << "HEADER OUT: " << header << endl;
   recv_packet (server, &header, sizeof header);
   log << "HEADER IN : " << header << endl;
   if (header.command != cix_command::ACK) {
      log << "Server failed to remove " << filename << endl;
   }
}

void usage() {
   cerr << "Usage: " << log.execname() << " [host] [port]" << endl;
   throw cix_exit();
}

void set_command_filename(string& line, string& command, string& name){
   // Trim line
   const auto strBegin = line.find_first_not_of(' ');
   if (strBegin == std::string::npos) return;  
   const auto strEnd = line.find_last_not_of(' ');
   const auto strRange = strEnd - strBegin + 1;
   line = line.substr(strBegin, strRange);

   size_t found = line.find(" ");
   if (found != string::npos) {
      command = line.substr(0, found);
      name = line.substr(found + 1);
   } else {
      command = line;
   }
}

int main (int argc, char** argv) {
   log.execname (basename (argv[0]));
   log << "starting" << endl;
   vector<string> args (&argv[1], &argv[argc]);
   if (args.size() > 2) usage();
   string host = get_cix_server_host (args, 0);
   in_port_t port = get_cix_server_port (args, 1);
   log << to_string (hostinfo()) << endl;
   try {
      log << "connecting to " << host << " port " << port << endl;
      client_socket server (host, port);
      log << "connected to " << to_string (server) << endl;
      for (;;) {
         string line;
         string command = "";
         string filename = "";
         getline (cin, line);
         if (cin.eof()) throw cix_exit();
         log << "command " << line << endl;
         set_command_filename(line, command, filename);
         const auto& itor = command_map.find (command);
         cix_command cmd = itor == command_map.end()? 
         cix_command::ERROR : itor->second;
         switch (cmd) {
            case cix_command::EXIT:
              throw cix_exit();
              break;
            case cix_command::HELP:
              cix_help();
              break;
            case cix_command::LS:
              cix_ls (server);
              break;
            case cix_command::GET:
               if (filename == ""){
                  log << "Error: invalid filename." << endl;
                  break;
               }
               cix_get(server, filename);
              break;
            case cix_command::PUT:
               cix_put(server, filename);
               break;
            case cix_command::RM:
               cix_rm(server, filename);
               break;
            default:
            log << line << ": invalid command" << endl;
            break;
         }
      }
   }catch (socket_error& error) {
      log << error.what() << endl;
   }catch (cix_exit& error) {
      log << "caught cix_exit" << endl;
   }
   log << "finishing" << endl;
   return 0;
}

