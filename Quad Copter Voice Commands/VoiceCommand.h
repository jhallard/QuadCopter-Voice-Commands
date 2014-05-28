#ifndef VOICECOMMAND_H 
#define VOICECOMMAND_H 
  
#include <WinSock2.h> 
#include <ad.h> 
#include <string> 
#include <iostream> 
#include <fstream> 
#include <pocketsphinx.h> 
#include <conio.h> 
#include <ctime> 
#include <err.h> 
#include <unordered_map> 
#include <vector> 
#include "ConfigNullException.h" 
#include "DecoderNullException.h" 
  
#define MODELDIR "../model/" 
#define UTTID    "command" 
#define SAMPLESPERSECOND 16000 
#define TIME             10000 
#define INPUTKEY         107 
#define BUFFERSIZE       (SAMPLESPERSECOND * (TIME / 1000)) 
  
class VoiceCommand 
{ 
  
private: 
  
    // member fields 
    ad_rec_t * audioRecorder; // recording struct 
    SOCKET *socket;         // Network connection 
    ps_decoder_t* decoder;      // Decodes the recording 
    cmd_ln_t* config;           // Configuration of voice model and dictionary to use. 
    std::string modelDir;       // Location of model folder. 
    int16 buffer[BUFFERSIZE];       // Buffer that stores recording 
    std::unordered_map<std::string, int> numberWords;         // Numbers in word form and their corresponding integer value 
  
    // utility functions 
    std::string recognizeCommand(int32); 
    int32 recordCommand(); 
    bool convertStringCommandToIntCommand(std::string); 
    int convertStringToNumber(std::vector<std::string>); 
    void loadNumberWords(); 
    bool sendCommand(); 
  
    //struct 
    struct copterCommand { 
        int copter; 
        int command; 
    } qcCommand; 
  
public: 
  
    VoiceCommand(SOCKET *, std::string); 
    //~VoiceCommand(); 
    void startVoiceCommand(); 
  
  
  
  
  
  
}; 
  
#endif