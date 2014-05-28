#ifndef CONFIG_NULL_EXCEPTION_H 
#define CONFIG_NULL_EXCEPTION_H 
  
#include <iostream> 
#include <exception> 
#include <stdexcept> 
  
class ConfigNullException : public std::runtime_error { 
public: 
    ConfigNullException() 
        : std::runtime_error("Error with configuration of acoustic and language model.") 
    { } 
  
    const char* what() const throw() { 
        return std::runtime_error::what(); 
    } 
}; 
  
  
#endif


  //  // Use the generic English acoustic and language model, and the cmu07a dictionary 
  //  config = cmd_ln_init(NULL, ps_args(), true,  
  //      "-hmm", MODELDIR "hmm/en_us2",                          // Directory of accoustic model 
  //      "-lm", MODELDIR "lm/en_US/an4.ug.lm.DMP",               // Directory of language model  "-lm", MODELDIR "lm/en_US/en_us.lm.dmp",  
  //      "-dict", MODELDIR "lm/en_US/an4.dic", NULL);
		////"-dict", MODELDIR "lm/en_US/cmu07a.dic", NULL);         // Directory of dictionary 