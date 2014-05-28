#ifndef DECODER_NULL_EXCEPTION_H 
#define DECODER_NULL_EXCEPTION_H 
  
#include <iostream> 
#include <exception> 
#include <stdexcept> 
  
class DecoderNullException : public std::runtime_error { 
public: 
    DecoderNullException() 
        : std::runtime_error("Error with creating the decoder.") 
    { } 
  
    const char* what() const throw() { 
        return std::runtime_error::what(); 
    } 
}; 
  
  
#endif