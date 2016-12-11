// $Id: file_sys.h,v 1.5 2016-04-07 13:36:11-07 - - $

#ifndef __INODE_H__
#define __INODE_H__

#include <exception>
#include <iostream>
#include <memory>
#include <map>
#include <vector>
using namespace std;

#include "util.h"

/**
 *  Enum class file_type
 *  PLAIN_TYPE: For an inode instance to store strings
 *  DIRECTORY_TYPE: For an inode instance to store inode
 */
enum class file_type {PLAIN_TYPE, DIRECTORY_TYPE};

/**
 * Declaration of classes.
 */
class inode;
class base_file;
class plain_file;
class directory;

class file_error;
class plain_file;
class directory;
class base_file;

/**
 *  Declaration of type alias.
 */
using inode_ptr = shared_ptr<inode>;
using base_file_ptr = shared_ptr<base_file>;
using plain_file_ptr = shared_ptr<plain_file>;
using directory_ptr = shared_ptr<directory>;

/**
 *  Declaration of functions.
 */
ostream& operator<< (ostream&, file_type);




/* CLASS */
/**
 * Class file_error
 * Custom exception class, which really don't change anything.
 * Should simplify the code using rumtime exception.
 */
class file_error: public runtime_error {
public:
    explicit file_error (const string& what);
};

/**
 *  Class inode_state
 *  An inconvenient class.
 *  This class maintain the state of the simulated file system.
 *
 *  Fields:
 *  shared_ptr<inode> root  This class always points to the root directory,
 *  shared_ptr<inode> cwd   and the current working directory.
 *  string prompt_          Display prompt. Probably not a good naming convention.
 *
 *  Constructors:
 *  Default - Singleton. Creates the root directory.
 *  Copy constrouctor and assignment operator are suppressed.
 *
 *  Method:
 *  prompt()                Bad convention. fix: get_prompt(). Returns prompt string.
 */
class inode_state {
    friend class inode;
    friend ostream& operator<< (ostream& out, const inode_state&);
private:
    inode_ptr root {nullptr};
    inode_ptr cwd {nullptr};
    string prompt_ {"% "};
public:
    // ♠ Implemented in cpp
    inode_state();
    // ♥ In line methods
    inode_state& operator= (const inode_state&) = delete;
    inode_state (const inode_state&) = delete;
    const string& prompt(){ return prompt_; };
    void set_prompt(string toSet){ prompt_ = toSet; };
    void set_cwd(inode_ptr toSet){ cwd = toSet; };
    inode_ptr get_cwd(){ return cwd; };
    inode_ptr get_root(){ return root; };
};

/**
 *  Class inode
 *  Instances of this class are a simulated file or a direcotry.
 *
 *  Fields:
 *  static int next_inode_nr        Class variable. Increament by 1 when an instance is created.
 *  int inode_nr                    Unique number for all instance.
 *  shared_ptr<base_file> contents  This field holds the data, either a direcotry or strings.
 *
 *  Constrcutors:
 *  Default - supressed.
 *  (enum class file_type)  Create inode with contents be either class directory or plain_file.
 *
 *  Methods:
 *  get_size()  Returns the sum file character or number of dirents.
 */
class inode {
    friend class inode_state;
private:
    static int next_inode_nr;
    int inode_nr;
    base_file_ptr contents;
    file_type this_type;
public:
    // ♠ Implemented in cpp
    inode (file_type type);
    string get_full_path();
    size_t get_size();
    string get_name();
    void print_contents(bool recursive);
    directory_ptr get_directory_access ();
    plain_file_ptr get_plain_file_access ();
    
    // ♥ In line methods
    int get_inode_nr() const { return inode_nr; };
    file_type get_this_type(){ return this_type; };

};


/**
 *  Class base_file
 *  This an abstract class.
 *  See details in Class directory and plain_file.
 */
class base_file {
protected:
    base_file() = default;
public:
    base_file (const base_file&) = delete;
    base_file& operator= (const base_file&) = delete;
    virtual ~base_file() = default;
    virtual size_t size() const = 0;
    virtual const wordvec& readfile() const = 0;
    virtual void writefile (const wordvec& newdata) = 0;
    virtual void remove (const string& filename, bool recursive) = 0;
    virtual inode_ptr mkdir (const string& dirname, inode_ptr parent) = 0;
    virtual inode_ptr mkfile (const string& filename, const string& content) = 0;
};

/**
 *  Class plain_file
 *  This class is for the inode content field.
 *
 *  Fields:
 *  vector<string> data     For holding string data.
 *
 *  Methods:
 *  readfile()      Basically get_data();
 *  writefile()     Basically set_data();
 *  remove()        Should not remove a directory on a file. throw error
 *  mkdir()         Should not make a directory on a file. throw error
 *  mkfile()        Should not make file on a file. throw error
 */
class plain_file: public base_file {
private:
    wordvec data;
public:
    // ♥ In line methods
    inode_ptr mkdir (const string& dirname, inode_ptr parent) override { throw file_error("Is a file");};
    inode_ptr mkfile (const string& filename, const string& content) override{ throw file_error("Is a file");};
    void remove (const string& filename, bool recursive) override{ throw file_error("Is a file"); };
    size_t size() const override { return this->data[1].length() -1; };
    string get_name(){ return data[0]; };
    const wordvec& readfile() const override { return data; };
    void writefile (const wordvec& newdata) override { data = newdata; };
};

/**
 *  Class directory
 *  This class is for the inode content field.
 *
 *  Fields:
 *  map<string,inode_ptr> dirents     For holding {title:inode}.
 *
 *  Methods:
 *  readfile()          Should not readfile from a directory. throw error
 *  writefile()         Should not writefile to a direcotry. throw error
 *  remove(name)        Remove all dirents iff dirents are (.) and (..)
 *  mkdir(name)         Creates a new directory under current directory with (.) and (..) in it.
 *  mkfile(name)        Create a new empty file with given name. throw error if name exists.
 */
class directory: public base_file {
private:
    map<string,inode_ptr> dirents;
    string name;
public:
    // ♠ Implemented in cpp
    void remove (const string& filename, bool recursive) override;
    inode_ptr mkdir (const string& dirname, inode_ptr parent) override;
    inode_ptr mkfile (const string& filename, const string& content) override;
    plain_file_ptr get_file (string file_name);
    directory_ptr get_dir_in_dirents (string dir_name);
    inode_ptr get_inode_in_dirents(string dir_name);
    
    // ♥ In line methods
    const wordvec& readfile() const override { throw file_error("Is a directory"); };
    void writefile (const wordvec& newdata) override { throw file_error("Is a directory"); };
    size_t size() const override { return dirents.size(); };
    string get_name(){ return name; };
    void insert_dirent(string name, inode_ptr toInsert){ dirents.insert(std::pair<string, inode_ptr>(name, toInsert)); };
    void set_name(string toSet) { name = toSet; };
    const map<string,inode_ptr> get_dirents() { return dirents; };
    bool check_filename(const string& filename) { return this->dirents.find(filename) != this->dirents.end(); };
};

#endif

