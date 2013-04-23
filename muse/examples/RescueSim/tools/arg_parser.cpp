#ifndef ARG_PARSER_CPP
#define ARG_PARSER_CPP

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

#include "arg_parser.h"

void 
arg_parser::get_arg_array(arg_record ptr[]) {

  array_of_arg_records = ptr;
  int i = 0;
  
  for (;;) {
    if (array_of_arg_records[i].arg_text == NULL) {
      break;
    } else {
      i++;
      if (i > 100) {
	// If you got into this conditional, you probably forgot
	// to put "arg_parser::last_arg" in your array.  If not,
	// increase the number above and recompile
	std::cerr << "More than 100 arguments?\n";
	exit(-1);
      }
    }
  }
  num_args = i;
}


bool
arg_parser::check_args(int &argc, char *argv[], bool caxoe) {
  // complain_and_exit_on_error defaults to true...

  int i,j;
  
  // This loop cycles through the arguments.
  for (i = 1; i < argc ; i++) {
    // This loop compares the arguments passed in with those we're
    // checking them against
    bool matched_one = false;
    for (j = 0; j < num_args; j++) {
      // the first check is necessary because argc can change during
      // execution...
      switch (array_of_arg_records[j].type) {
      case BOOLEAN:{
	
	if (i < argc && strcmp(argv[i], array_of_arg_records[j].arg_text) == 0) {
	  // They matched - let's read in the data
	  matched_one = true;
	  // Argc is passed by reference!
	  remove_arg(i, argc, argv);
	  *(bool*)(array_of_arg_records[j].data) = true;
	}
	break;
      }
      case INTEGER:{
	if (i < argc && strcmp(argv[i], array_of_arg_records[j].arg_text) == 0) {
	  matched_one = true;
	  remove_arg( i, argc, argv );
	  *(int*)(array_of_arg_records[j].data) = atoi(argv[i]);
	  remove_arg( i, argc, argv );
	}
	break;
      }
      case STRING:{
	// Argc is passed by reference!	      
	if (i < argc && strcmp(argv[i], array_of_arg_records[j].arg_text) == 0) {
	  // They matched - let's read in the data
	  matched_one = true;
	  remove_arg(i, argc, argv);
	  *(char**)(array_of_arg_records[j].data) = argv[i];
	  remove_arg(i, argc, argv);
	}
	break;
      }
      case STRING_LIST:{
	char *substring = NULL;
	if (i < argc && strcmp(argv[i], array_of_arg_records[j].arg_text) == 0){
	  remove_arg(i, argc, argv);
	  substring = argv[i];
	}
	else{
	  int character;
	  bool one_did_not_match = false;
	  for( character = 0; 
	       character < (int) strlen(array_of_arg_records[j].arg_text); 
	       character++ ){
	    if( array_of_arg_records[j].arg_text[character] != argv[i][character] ){
	      one_did_not_match = true;
	    }
	  }	  
	  if( one_did_not_match == false ){
	    substring = argv[i] + (strlen( array_of_arg_records[j].arg_text )*sizeof(char));
	  }
	}
	if( substring != NULL ){
	  matched_one = true;
	  // ((dl_list<char> *)(array_of_arg_records[j].data))->append(substring);
	  remove_arg(i, argc, argv);
	}
	break;
      }
      case DOUBLE:{
	if (i < argc && strcmp(argv[i], array_of_arg_records[j].arg_text) == 0) {
	  matched_one = true;
	  remove_arg( i, argc, argv );
	  *(double*)(array_of_arg_records[j].data) = atof(argv[i]);
	  remove_arg( i, argc, argv );
	}
	break;
      }
#ifndef _WINDOWS
      case LONG_LONG:{
	if (i < argc && strcmp(argv[i], array_of_arg_records[j].arg_text) == 0) {
	  matched_one = true;
	  remove_arg( i, argc, argv );
	  *(long long*)(array_of_arg_records[j].data) = atoll(argv[i]);
	  remove_arg( i, argc, argv );
	}
	break;
      }   
#endif
      default:
	std::cerr << "Invalid arg type in arg array!\n";
        exit(-1);
      }
      
      if( matched_one == true ){
	// we need to reprocess all of the args, to be safe...
	j = 0;
	i = 0;
	matched_one = false;
      }
    } // for j
  } // for i
  
  return check_remaining( argc, argv, caxoe );
}


void 
arg_parser::remove_arg( int arg_to_remove, int &argc, char **argv ){
  // let's shift the rest of the arguments around...
  int i;
  for( i = arg_to_remove; i < argc - 1; i++ ){
    argv[i] = argv[i+1];
  }
  argv[i] = NULL;
  
  argc--;
}


bool 
arg_parser::check_remaining(int argc, char **argv, 
			    bool complain_and_exit_on_error ){
  int i;
  
  for(i = 0; i < argc; i++){
    
    if( strcmp(argv[i], "-help") == 0 ){
      std::cout << "Valid arguments are:\n";
      std::cout << *this << std::endl;
      exit( 0 );
    }
    
    if( argv[i][0] == '-' ){
      // Then someone passed in an illegal argument!
      if(  complain_and_exit_on_error == true) {
	std::cerr << "Invalid argument \"" << argv[i] << "\"\n";
	std::cerr << "Valid arguments: \n";
	std::cerr << *this << std::endl;
	exit( -1 );
      }
      return false;
    }
  }

  return true;
}

#endif
