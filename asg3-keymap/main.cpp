// $Id: main.cpp,v 1.8 2015-04-28 19:23:13-07 - - $

#include <cstdlib>
#include <exception>
#include <iostream>
#include <string>
#include <unistd.h>
#include <cassert>
#include <regex>
#include <fstream>

using namespace std;

#include "listmap.h"
#include "xpair.h"
#include "util.h"

using str_str_map = listmap<string,string>;
using str_str_pair = str_str_map::value_type;

void scan_options (int argc, char** argv) {
   opterr = 0;
   for (;;) {
      int option = getopt (argc, argv, "@:");
      if (option == EOF) break;
      switch (option) {
         case '@':
            traceflags::setflags (optarg);
            break;
         default:
            complain() << "-" << (char) optopt << ": invalid option"
            << endl;
            break;
      }
   }
}

string form_pair_string (str_str_pair& it){
   string message = it.first;
   message.append(" = ");
   message.append(it.second);
   message.append("\n");
   return message;
}

string form_pair_string (const string& key, const string& val){
   string message = key;
   message.append(" = ");
   message.append(val);
   message.append("\n");
   return message;
}

string analyze_pair(string key, string val, str_str_map& to_do){
   string message = "";
   // Case: (=): Print all pairs
   if (key.empty() && val.empty()) {
      for (auto it = to_do.begin(); it != to_do.end(); ++it) {
         message.append(form_pair_string(*it));
      }
      return message;
   }
   
   // Case: (Key =): Delete target pair
   if (!key.empty() && val.empty()) {
      for (auto it = to_do.begin(); it != to_do.end(); ++it) {
         if (it->first.compare(key) == 0) {
            to_do.erase(it);
            break;
         }
      }
      return message;
   }
   
   // Case: (= value): Print target keys
   if (key.empty() && !val.empty()) {
      for (auto it = to_do.begin(); it != to_do.end(); ++it) {
         if (it->second.compare(val) == 0) {
            message.append(form_pair_string(*it));
         }
      }
      return message;
   }
   
   // Case: (Key = value): Assign target pair
   str_str_pair to_insert (key, val);
   to_do.insert(to_insert);
   return message;
}

string analyze_line(string& to_analyze, str_str_map& to_operate){
   regex regex_comment {R"(^\s*(#.*)?$)"};
   regex regex_key_value {R"(^\s*(.*?)\s*=\s*(.*?)\s*$)"};
   regex regex_trimmed {R"(^\s*([^=]+?)\s*$)"};
   smatch result;
   string message = "";
   
   // Case: (#)
   if (regex_search (to_analyze, result, regex_comment)) {
      // Do nothing
   } else if (regex_search (to_analyze, result, regex_key_value)) {
      // Case: (key=) (key=val) (=val)
      const string& key = result[1];
      const string& val = result[2];
      message.append(analyze_pair(key, val, to_operate));
      
   } else if (regex_search (to_analyze, result, regex_trimmed)) {
      // Case: (key)
      const string& key = result[1];
      const string& val = to_operate.find(key);
      if(!val.empty()){
         message.append(form_pair_string(key, val));
      } else {
         message.append(key);
         message.append(": key not found");
         message.append("\n");
      }
      // print target pair
   } else {
      assert (false and "This can not happen.");
      throw domain_error("Analyze line fail");
   }
   return message;
}

void scan_cin (bool& read_cin, str_str_map& to_operate, int& line_nr){
   while (read_cin) {
      const string name = "-: ";
      string buffer;
      // Read line
      getline (cin, buffer);
      
      // End of file, terminate the loop.
      if (cin.eof()) break;
      
      // Print the user input on the terminal
      cout << "-: " << ++line_nr << ": " << buffer << endl;
      
      // Analysis input and print result
      cout << analyze_line(buffer, to_operate);
      
   }
   read_cin = false;
}

int main (int argc, char** argv) {
   sys_info::set_execname (argv[0]);
   scan_options (argc, argv);
   str_str_map test;
   bool read_cin = argc ==1? true : false;
   int line_nr = 0;
   string buffer;
   string name;

   
   try {
      // No input files, scan cin.
      scan_cin(read_cin, test, line_nr);

      // For each input files, scan file
      for (int i = 1; i < argc; i++) {
         // Format name
         name = argv[i];
         name.append(": ");
         
         // For scan_cin when input is '-'
         if (name.compare("-: ") == 0) {
            read_cin = true;
            scan_cin(read_cin, test, line_nr);
            continue;
         }
         
         // Open file
         ifstream in_file;
         in_file.open(argv[i]);
         
         // Check file
         if (in_file.fail()) {
            cerr << argv[0]<< ": "<<argv[i];
            cerr << ": No such file or directory" << endl;
            return 1;
         }
         
         // Read lines
         while (getline(in_file, buffer)) {
            cout << name << ++line_nr << ": " << buffer << endl;
            cout << analyze_line(buffer, test);
         }
         in_file.~basic_ifstream();
   } // Repeat until all files are read
      
      
   } catch (domain_error& e){
      cerr << e.what() << endl;
      return 1;
   }
   
   cout << "EXIT_SUCCESS" << endl;
   return EXIT_SUCCESS;
}

