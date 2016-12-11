// $Id: main.cpp,v 1.9 2016-01-14 16:16:52-08 - - $

#include <cstdlib>
#include <iostream>
#include <string>
#include <utility>
#include <unistd.h>

using namespace std;

#include "commands.h"
#include "debug.h"
#include "file_sys.h"
#include "util.h"


/**
 * This function scans command line arugment and activates the debug tools
 */
void scan_options (int argc, char** argv) {
    opterr = 0;
    for (;;) {
        int option = getopt (argc, argv, "@:");
        if (option == EOF) break;
        switch (option) {
            case '@':
                debugflags::setflags (optarg);
                break;
            default:
                complain() << "-" << static_cast<char> (option)
                << ": invalid option" << endl;
                break;
        }
    }
    if (optind < argc) {
        complain() << "operands not permitted" << endl;
    }
}


int main (int argc, char** argv) {
    // Basic settings that we don't care
    execname (argv[0]);
    cout << boolalpha;
    cerr << boolalpha;
    cout << argv[0] << " build " << __DATE__ << " " << __TIME__ << endl;
    
    // Enable debug tool if needed.
    scan_options (argc, argv);
    bool need_echo = want_echo();
    
    // Use default constructor to create state of inode_state
    inode_state state;
    
    try {
        for (;;) {
            try {
                // prompt() is actually get_prompt(), bad naming convention practice.
                cout << state.prompt();
                
                // Prepare to get input
                string line;
                getline (cin, line);
                
                // End of file, terminate the program.
                if (cin.eof()) {
                    // Bad if-statement. Should always include {}
                    if (need_echo) cout << "^D";
                    cout << endl;
                    DEBUGF ('y', "EOF");
                    break;
                }
                
                // Print the user input on the terminal
                if (need_echo) cout << line << endl;
                
                // Read input by splitting and searching.
                try {
                    wordvec words = split (line, " \t");
                    DEBUGF ('y', "words = " << words);
                    command_fn fn = find_command_fn (words.at(0));
                    
                    // Excuate function or system throw command_error.
                    fn (state, words);
                } catch (std::out_of_range) {
                    // Probably an empty input.
                }
                
                
            } catch (command_error& error) {
                // Print error message
                complain() << error.what() << endl;
            }
        }
    } catch (ysh_exit&) {
        // This catch intentionally left blank.
    }
    
    return exit_status_message();
}

