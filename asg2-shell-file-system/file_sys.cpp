// $Id: file_sys.cpp,v 1.5 2016-01-14 16:16:52-08 - - $

#include <iostream>
#include <stdexcept>
#include <unordered_map>

using namespace std;

#include "debug.h"
#include "file_sys.h"

int inode::next_inode_nr {1};

/* FILE_TYPE */
struct file_type_hash {
    size_t operator() (file_type type) const {
        return static_cast<size_t> (type);
    }
};

file_error::file_error (const string& what): runtime_error (what) {
}


/* INODE_STATE */
inode_state::inode_state() {
    root = make_shared<inode>(file_type::DIRECTORY_TYPE);
    root->get_directory_access()->set_name("/");
    root->get_directory_access()->insert_dirent(".", root);
    root->get_directory_access()->insert_dirent("..", root);
    cwd = root;
}

/* INODE */
inode::inode (file_type type):
inode_nr (next_inode_nr++),
this_type(type){
    switch (this_type) {
        case file_type::DIRECTORY_TYPE:
            contents =  make_shared<directory>();
            break;
        case file_type::PLAIN_TYPE:
            contents = make_shared<plain_file>();
            break;
    }
}

size_t inode::get_size(){
    switch (this_type) {
        case file_type::DIRECTORY_TYPE:
            return dynamic_pointer_cast<directory> (contents)->directory::size();
            break;
        case file_type::PLAIN_TYPE:
            return dynamic_pointer_cast<plain_file> (contents)->plain_file::size();
            break;
    }
}
string inode::get_name(){
    switch (this_type) {
        case file_type::DIRECTORY_TYPE:
            return this->get_directory_access()->get_name();
            break;
        case file_type::PLAIN_TYPE:
            return this->get_plain_file_access()->get_name();
            break;
    }
}


string inode::get_full_path(){
    if (this->get_name() == "/") {
        return "/";
    }
    string parent_path = "";
    wordvec reverse_build {};
    reverse_build.push_back(this->get_name());
    inode_ptr temp = this->get_directory_access()->get_inode_in_dirents("..");
    while (temp->get_name() != "/"){
        reverse_build.push_back(temp->get_name());
        temp = temp->get_directory_access()->get_inode_in_dirents("..");
    }
    for (auto it = reverse_build.rbegin(); it != reverse_build.rend(); it++) {
        parent_path.append("/");
        parent_path.append(it->data());
    }
    return parent_path;
}


shared_ptr<directory> inode::get_directory_access() {
    if (this_type == file_type::PLAIN_TYPE) {
        throw file_error("Is a file.");
    }
    shared_ptr<directory> dir_ptr = dynamic_pointer_cast<directory> (contents);
    if (dir_ptr == nullptr) {
        throw file_error("cast_directory fail: dir nullptr");
    }
    return dir_ptr;
}

shared_ptr<plain_file> inode::get_plain_file_access() {
    if (this_type == file_type::DIRECTORY_TYPE) {
        throw file_error("Is a directory.");
    }
    shared_ptr<plain_file> file_ptr = dynamic_pointer_cast<plain_file> (contents);
    if (file_ptr == nullptr) {
        throw file_error("cast_directory fail: direnullptr");
    }
    return file_ptr;
}

void inode::print_contents(bool recursive){
    cout << this->get_full_path() << ":" << endl;
    if (this_type == file_type::PLAIN_TYPE) {
        cout << this->get_name() << endl;
    } else {
        map<string,inode_ptr> dirents = this->get_directory_access()->get_dirents();
        for (auto it = dirents.cbegin(); it != dirents.cend(); it++) {
            cout << this->inode_nr << "\t" << it->second->get_size();
            if (it->first == "." || it->first == "..") {
                cout << "\t" << it->first << endl;
            } else if (it->second->get_this_type() == file_type::DIRECTORY_TYPE){
                cout << "\t" << it->first << "/" << endl;
            } else {
                cout << "\t" << it->first << endl;
            }
        }
    }
    if (recursive) {
        map<string,inode_ptr> dirents = this->get_directory_access()->get_dirents();
        for (auto it = dirents.cbegin(); it != dirents.cend(); it++) {
            if ( it->second->get_this_type() == file_type::DIRECTORY_TYPE) {
                if (it->first != "." && it->first != "..") {
                    it->second->print_contents(true);
                }
            }
        }
    }
    
}


/* DIRECTORY */

void directory::remove (const string& filename, bool recursive) {
    if(!check_filename(filename)){
        throw file_error("No such file or directory");
    }
    inode_ptr toDelete = this->dirents.at(filename);
    if (toDelete->get_this_type() == file_type::PLAIN_TYPE) {
        this->dirents.erase(filename);
        return;
    }
    unsigned long check = toDelete->get_directory_access()->dirents.size();
    if (check == 2) {
        toDelete->get_directory_access()->dirents.clear();
        this->dirents.erase(this->dirents.find(filename));
    } else if (!recursive) {
        throw file_error("Directory is not empty");
    }
    
    if (recursive) {
        for (auto it = toDelete->get_directory_access()->dirents.cbegin(); it != toDelete->get_directory_access()->dirents.cend(); it++) {
            if (it->first != "." && it->first != "..") {
                if (it->second->get_this_type() == file_type::PLAIN_TYPE) {
                    toDelete->get_directory_access()->dirents.erase(it->first);
                } else {
                    toDelete->get_directory_access()->remove(it->first, true);
                }
            }
        }
        toDelete->get_directory_access()->dirents.clear();
        this->dirents.erase(this->dirents.find(filename));
    }
}


/**
 * This method makes a directory under the caller's dirents.
 *
 * @return nullptr
 */
inode_ptr directory::mkdir (const string& dirname, inode_ptr parent) {
    if(check_filename(dirname)){
        throw file_error ("File exists");
    }
    inode_ptr dir = make_shared<inode>(file_type::DIRECTORY_TYPE);
    dir->get_directory_access()->set_name(dirname);
    dir->get_directory_access()->dirents.insert(std::pair<string, inode_ptr>(".", dir));
    dir->get_directory_access()->dirents.insert(std::pair<string, inode_ptr>("..", parent));
    this->dirents.insert(std::pair<string, inode_ptr>(dirname, dir));
    return nullptr;
}

inode_ptr directory::mkfile (const string& filename, const string& contents) {
    if(check_filename(filename)){
        throw file_error ("File exists");
    }
    inode_ptr file = make_shared<inode>(file_type::PLAIN_TYPE);
    wordvec data {};
    data.push_back(filename);
    data.push_back(contents);
    file->get_plain_file_access()->writefile(data);
    this->dirents.insert(std::pair<string, inode_ptr>(filename, file));
    return nullptr;
}

directory_ptr directory::get_dir_in_dirents (string dir_name){
    if(!check_filename(dir_name)){
        throw file_error ("No such file or directory");
    }
    return this->dirents.find(dir_name)->second->get_directory_access();
}

inode_ptr directory::get_inode_in_dirents(string dir_name){
    if(!check_filename(dir_name)){
        throw file_error ("No such file or directory");
    }
    inode_ptr target = dirents.at(dir_name);
    return target;
}


/* STRING OUT */
// enum file_type
ostream& operator<< (ostream& out, file_type type) {
    static unordered_map<file_type,string,file_type_hash> hash {
        {file_type::PLAIN_TYPE, "PLAIN_TYPE"},
        {file_type::DIRECTORY_TYPE, "DIRECTORY_TYPE"},
    };
    return out << hash[type];
}

// inode_state
ostream& operator<< (ostream& out, const inode_state& state) {
    out << "inode_state: root = " << state.root
    << ", cwd = " << state.cwd;
    return out;
}

