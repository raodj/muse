#ifndef ARG_PARSER_H
#define ARG_PARSER_H

//---------------------------------------------------------------------------
//
// Copyright (c) Miami University, Oxford, OHIO.
// All rights reserved.
//
// Miami University (MU) makes no representations or warranties about
// the suitability of the software, either express or implied,
// including but not limited to the implied warranties of
// merchantability, fitness for a particular purpose, or
// non-infringement.  MU shall not be liable for any damages suffered
// by licensee as a result of using, result of using, modifying or
// distributing this software or its derivatives.
//
// By using or copying this Software, Licensee agrees to abide by the
// intellectual property laws, and all other applicable laws of the
// U.S., and the terms of this license.
//
// Authors: Dhananjai M. Rao       raodm@muohio.edu
//
//---------------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <iostream>

class arg_parser {

// @BeginExternalProseDescription

// This file contains the declarations for a class which accepts a list of
// arguments with addresses of the variables they affect, and parses argv
// and argc to check for them.  Errors are dealt with as well.  The
// variables can either be boolean, char*, or dl_list<char*>;
// which type a variable is is determined by a value of the enumerated
// type arg_parser::arg_type.  

// Here is an example of the use of this class:
//
// bool arg1;      // These must be static or global scope...
// bool arg2;
// char *arg3;
//
// arg_parser::arg_record arg_list[] = {
//    { "-arg1", &arg1, arg_parser::BOOLEAN }, 
//    { "-arg2", &arg2, arg_parser::BOOLEAN },
//    { "-arg3", &arg3, arg_parser::STRING },
//    { NULL, NULL }
// };
//
//
// int main( int argc, char *argv[] ){
//     arg1 = true;    // default initialization must occur before the
//     arg2 = false;   // arg_parser is called!
//     arg3 = NULL;
//
//     arg_parser ap( arg_list );
//     ap.check_args( argc, argv );

// @EndExternalProseDescription

  friend std::ostream &operator<<(std::ostream &, const arg_parser &);

public:
  enum arg_type {BOOLEAN, INTEGER, STRING, STRING_LIST, DOUBLE, LONG_LONG};

  struct arg_record {
    const char *arg_text;
    const char *arg_help;
    void *data;
    arg_type type;
  };

  arg_record last_arg;
  
  arg_parser() {
    array_of_arg_records = NULL;
    num_args = 0;
  }
  
  arg_parser(arg_record *record_ptr) {
    get_arg_array(record_ptr);
  }
  
  ~arg_parser() {}
  
  void get_arg_array(arg_record[]);
  
  // The int is the number of args (argc), the char ** is argv, and
  // the bool is whether to complain and exit on error or not...
  bool check_args(int &, char **, bool = true );
  
private:
  arg_record *array_of_arg_records;
  int num_args;
  
  // This method is used to pull arguments from the command line as 
  // they're processed...
  void remove_arg( int arg_to_remove, int &argc, char **argv);
  
  // This method checks the arguments passed in to see if there are any
  // in the form "-blah" and complains, if the global var 
  // "complain_and_exit_on_error" is set to true.
  bool check_remaining( int argc, char **argv,
			bool complain_and_exit_on_error);
};

inline 
std::ostream &operator<<(std::ostream &os, const arg_parser &ap){
  const int num_spaces = 3;
  const int indentation = 2;
  
  // calculate the length of the longest argument
  int i = 0;
  int maxlen = 0;
  while( ap.array_of_arg_records[i].arg_text != NULL ){
    if( (int) strlen(ap.array_of_arg_records[i].arg_text) > maxlen ){
      maxlen = (int) strlen(ap.array_of_arg_records[i].arg_text);
    }
    i++;
  }
  if (maxlen < 12) {
    maxlen = 12;
  }
  
  // print the argument array
  int j;
  i = 0;
  while( ap.array_of_arg_records[i].arg_text != NULL ){
    
    // indent the proper amount
    for( j = 0; j < indentation; j++ ){
      os << " ";
    }
    
    // here is the argument
    os << ap.array_of_arg_records[i].arg_text;
    
    // print out the padding - leave num_spaces spaces between args and 
    // help text...
    for( j = 0; j < (int) (maxlen - strlen(ap.array_of_arg_records[i].arg_text)
	   + num_spaces); j++ ){
      os << " ";
    }
    
    // here is the help string.
    os << ap.array_of_arg_records[i].arg_help << std::endl;
    i++;
  }
  
//   for( j = 0; j < indentation; j++ ){
//     os << " ";
//   }
  
//   os << "-help";
//   for( j = 0; j < (int) (maxlen - strlen("-help")  + num_spaces) ; j++ ){
//     os << " ";
//   }
//   os << "print this message" << std::endl;
  
  return os;
}

#endif
