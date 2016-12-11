// $Id: commands.cpp,v 1.16 2016-01-14 16:10:40-08 - - $

#include "commands.h"
#include "debug.h"

command_hash cmd_hash {
    {"cat"   , fn_cat   },
    {"cd"    , fn_cd    },
    {"echo"  , fn_echo  },
    {"exit"  , fn_exit  },
    {"ls"    , fn_ls    },
    {"lsr"   , fn_lsr   },
    {"make"  , fn_make  },
    {"mkdir" , fn_mkdir },
    {"prompt", fn_prompt},
    {"pwd"   , fn_pwd   },
    {"rm"    , fn_rm    },
    {"rmr"   , fn_rmr   },
    {"#"     , fn_com   },
};

command_fn find_command_fn (const string& cmd) {
    // Note: value_type is pair<const key_type, mapped_type>
    // So: iterator->first is key_type (string)
    // So: iterator->second is mapped_type (command_fn)
    const auto result = cmd_hash.find (cmd);
    if (result == cmd_hash.end()) {
        throw command_error (cmd + ": no such function");
    }
    return result->second;
}

string get_parent_path(const wordvec& pathvec, bool from_root){
    string parent_path = from_root? "/" : "";
    for (int i = 0; i < ((int)pathvec.size())-1; i++) {
        parent_path.append(pathvec[i]);
        parent_path.append("/");
    }
    return parent_path;
}

inode_ptr trace_path (inode_state& state, const wordvec& words){
    // Get strings ready
    string toCheck = "";
    if (words.size() >=2) {
         toCheck = words[1];
    }
    wordvec path = split (toCheck, "/");
    
    // Set to cwd since no path provided
    if (path.size() == 0) {
        return state.get_cwd();
    }
    
    // Create variables
    directory_ptr access;
    
    // Get access
    access = toCheck.front() == '/'?
    state.get_root()->get_directory_access() :
    state.get_cwd()->get_directory_access();
    
    // Access path
    for (int i = 0; i < path.size()-1; i++) {
        access = access->get_dir_in_dirents(path[i]);
    }
    
    // Get destination inode_ptr
    return access->get_inode_in_dirents(path.back());
}

command_error::command_error (const string& what):runtime_error (what) {
}

int exit_status_message() {
    int exit_status = exit_status::get();
    cout << execname() << ": exit(" << exit_status << ")" << endl;
    return exit_status;
}

void fn_cat (inode_state& state, const wordvec& words){
    try{
        // Basic check
        if(words[1].back() == '/') {
            throw file_error("Is not a file.");
        }
        plain_file_ptr file = trace_path(state,words)->get_plain_file_access();
        cout << file->readfile()[1] << endl;
    } catch(file_error& error){
        cout << "cat: " << words[1] << ": "<< error.what() << endl;
    }
}

void fn_cd (inode_state& state, const wordvec& words){
    try{
        if (words.size() == 1) {
            state.set_cwd(state.get_root());
        } else {
            state.set_cwd(words[1] == "/"? state.get_root():trace_path(state, words));
        }
    } catch(file_error& error){
        cout << "cd: " << words[1] << ": "<< error.what() << endl;
    }
}

void fn_echo (inode_state& state, const wordvec& words){
    cout << word_range (words.cbegin() + 1, words.cend()) << endl;
}


void fn_exit (inode_state& state, const wordvec& words){
    throw ysh_exit();
}

void fn_ls (inode_state& state, const wordvec& words){
    try{
        trace_path(state,words)->print_contents(false);
    } catch(file_error& error){
        cout << "ls: " << words[1] << ": "<< error.what() << endl;
    }
}

void fn_lsr (inode_state& state, const wordvec& words){
    try{
        trace_path(state,words)->print_contents(true);
    } catch(file_error& error){
        cout << "lsr: " << words[1] << ": "<< error.what() << endl;
    }
}

void fn_make (inode_state& state, const wordvec& words){
    try {
        if ( words.size() < 2) {
            throw file_error("Missing file operand");
        }
        wordvec paths = split (words[1], "/");
        string parent_path_string = get_parent_path(paths, words[1].front() == '/');
        wordvec parent_path = words;
        parent_path[1] = parent_path_string;
        string content = "";
        bool check = false;
        for (int i = 2; i < words.size(); i++) {
            content.append(words[i]);
            content.append(" ");
            check = true;
        }
        if (!check) {
            content = " ";
        }
        trace_path(state, parent_path)->get_directory_access()->mkfile(paths.back(), content);
    } catch (file_error& error) {
        cout << "make: " << error.what() << endl;
    }
}

void fn_mkdir (inode_state& state, const wordvec& words){
    try {
        if (words.size() < 2) {
            throw file_error("Missing file operand");
        }
        
        wordvec paths = split (words[1], "/");
        string parent_path_string = get_parent_path(paths, words[1].front() == '/');
        wordvec parent_path = words;
        parent_path[1] = parent_path_string;
        inode_ptr parent = trace_path(state, parent_path);
        parent->get_directory_access()->mkdir(paths.back(), parent);
        
        
    } catch (file_error& error) {
        cout << "mkdir: " << error.what() << endl;
    }
}

void fn_prompt (inode_state& state, const wordvec& words){
    string new_prompt = "";
    for (int i = 1; i < words.size() ; i++) {
        new_prompt.append(words[i]);
        new_prompt.append(" ");
    }
    state.set_prompt(new_prompt);
    
    
}

void fn_pwd (inode_state& state, const wordvec& words){
    wordvec message_set;
    string message = "";
    inode_ptr temp = state.get_cwd();
    if (temp == state.get_root()) {
        message.append("/");
        cout << message << endl;
        return;
    }
    while (temp->get_name() != "/") {
        message_set.push_back(temp->get_name());
        temp = temp->get_directory_access()->get_inode_in_dirents("..");
    }
    for (int i = (int)message_set.size() -1 ; i >= 0;  i--) {
        message.append("/");
        message.append(message_set.at(i));
    }
    cout << message << endl;
}

void fn_rm (inode_state& state, const wordvec& words){
    try{
        if (words.size() != 2) {
            throw file_error("Missing file operand");
        }
        if (words[1] == "/" || words[1] == "." || words[1] == "..") {
            throw file_error("Invalid argument");
        }
        wordvec paths = split (words[1], "/");
        string parent_path_string = get_parent_path(paths, words[1].front() == '/');
        wordvec parent_path = words;
        parent_path[1] = parent_path_string;
        trace_path(state, parent_path)->get_directory_access()->remove(paths.back(), false);
    } catch (file_error& error){
        cout << "rm: " << error.what() << endl;
    }
}

void fn_rmr (inode_state& state, const wordvec& words){
    try{
        if (words.size() != 2) {
            throw file_error("Missing file operand");
        }
        if (words[1] == "/" || words[1] == "." || words[1] == "..") {
            throw file_error("Invalid argument");
        }
        wordvec paths = split (words[1], "/");
        string parent_path_string = get_parent_path(paths, words[1].front() == '/');
        wordvec parent_path = words;
        parent_path[1] = parent_path_string;
        trace_path(state, parent_path)->get_directory_access()->remove(paths.back(), true);
    } catch (file_error& error){
        cout << "rm: " << error.what() << endl;
    }
}

void fn_com (inode_state& state, const wordvec& words){
    // Does nothing
}
