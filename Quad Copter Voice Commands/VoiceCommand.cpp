#include "VoiceCommand.h" 
  
VoiceCommand::VoiceCommand(SOCKET *skt, std::string directory) 
{ 
    std::cout << "Initializing voice recognition\n\t"; 
    socket = skt; 
      
    //Disable INFO messages 
    err_set_logfp(NULL); 
    //err_set_logfile(NULL); 
    //err_set_debug_level(0); 
  
    /* Allocate buffers for audio recording. 
    * Note: For 1 second, 6 buffers are allocated each with 2500 samples*/
    audioRecorder = ad_open_sps_bufsize (SAMPLESPERSECOND, TIME); 
  
    // Use the generic English acoustic and language model, and the cmu07a dictionary 
    config = cmd_ln_init(NULL, ps_args(), true,  
        "-hmm", MODELDIR "hmm/en_us2",                          // Directory of accoustic model 
        "-jsgf", MODELDIR "lm/en_US/quad_com.jsgf",             // Directory of language model 
        "-dict", MODELDIR "lm/en_US/qc_com.dic", NULL);         // Directory of dictionary 
  
    // Error in configuring the model and/or dictionary. 
    if (config == NULL) 
        throw ConfigNullException(); 
  
    //Create the decoder (finds the words in a recording) 
    decoder = ps_init(config); 
      
    // Error creating the decoder 
    if (decoder == NULL) 
        throw DecoderNullException(); 
  
    // Load the word represenation of numbers 
    loadNumberWords(); 
  
    std::cout << std::endl; 
} 
  
std::string VoiceCommand::recognizeCommand(int32 numSamples) { 
    int success;                // Sphinx functions return negative values for errors 
      
    //Start the utterence 
    success = ps_start_utt(decoder, "command"); 
    if (success < 0) 
        return NULL; 
  
    //Process the voice  
    success = ps_process_raw(decoder, buffer, numSamples, false, true); 
    if (success < 0) 
        return NULL; 
  
    //Stop the utterence 
    success = ps_end_utt(decoder); 
    if (success < 0) 
        return NULL; 
  
    // Get the hypothesis 
    char const *hyp, *uttid; 
    int32 score; 
    hyp = ps_get_hyp(decoder, &score, &uttid); 
    if (hyp == NULL) 
        return ""; 
  
    return hyp; 
} 
  
/** Runs the voice recognition aspect of the program. Everything stems from this this one function. 
* It runs the loop until the user states that he/she wants to exit. 
*/
void VoiceCommand::startVoiceCommand() 
{ 
    bool exit = false;                      // Sentinel for the do-while loop 
    do { 
        int32 numSamples = 0;               //Number of samples recorded 
        std::cout << "Ready for command" << std::endl; 
        numSamples = recordCommand(); 
        if (numSamples == 0) 
            continue;                       // Empty so go to the top of the loop 
        // Get the hypothesis of what is assumed to have been heard 
        std::string hyp = recognizeCommand(numSamples); 
  
        if (hyp != "" ) { 
  
            //Confirm the command 
            std::cout << "Command: " + hyp +"\nConfirm Command?" << std::endl; 
            numSamples = recordCommand(); 
            std::string confirm = recognizeCommand(numSamples); 
            if (confirm == "yes" || confirm == "roger") { 
                std::cout << "Confirmed" << std::endl; 
                if (hyp == "exit")  
                    exit = true; 
                else { 
                    bool result = convertStringCommandToIntCommand(hyp); 
  
                    // Send the command if the conversion was successful 
                    if (result) 
                        result = sendCommand(); 
                } 
            } 
        } 
    } while(!exit); 
} 
  
/**Tells the OS to start and stop recording. Records for the time dictated by TIME in ms 
* @return       The number of samples recorded. 
*/
int32 VoiceCommand::recordCommand() { 
    if (_getch() == INPUTKEY) { 
  
        std::cout << "Recording..." << std::endl; 
  
        // Tell OS to start recording and begin timer 
        ad_start_rec(audioRecorder); 
        time_t startTime = time(NULL); 
  
        //Hold down key to record command for a max of TIME/1000 
        bool stop = false; 
        while(!stop && (difftime(time(NULL), startTime) <= (TIME/1000))) { 
            if (_kbhit() > 0 && _getch() == INPUTKEY) 
                stop = true; 
        } 
  
        //Move data to read buffer and stop recording 
        int32 numSamples = ad_read(audioRecorder, buffer, BUFFERSIZE); 
        ad_stop_rec(audioRecorder); 
        std::cout << "Recording ended" << std::endl; 
  
        return numSamples; 
    } 
    return 0; 
} 
  
bool VoiceCommand::convertStringCommandToIntCommand(std::string hyp) { 
    //Since here, it is assuming that it is all commands, so the first thing is the callsign. 
  
    //TODO: These are the few simple commands currently available. As the list grows, it would be better 
    // to place them on a file, and load them up that way so they're not hardcoded. 
    std::string commands[] = {"take", "hover", "move", "square", "figure", "scan", "stop"}; 
  
    //Find where the callsign ends. 
    int commandStart = -1;                                      //States where the actual command begins. -1 means it is not there 
    for (int i = 0; i < 10 && commandStart == -1; i++) { 
        int temp = hyp.find(commands[i]); 
        if (temp != std::string::npos) 
            commandStart = temp; 
    } 
    if (commandStart == -1) 
        return false; 
  
    // TODO: This is assuming that the callsign is "quebec charlie <number>". When different callsigns 
    // are implemented, it will be better to use a hash map that takes in the entire call sign and returns 
    // the value for that copter. 
    std::string callsign = hyp.substr(0, commandStart - 1); 
    std::vector<std::string> callsignNumber; 
  
    // Seperate the number from the callsign 
    while (callsign != "quebec charlie") { 
        int lastNumber = callsign.find_last_of(' '); 
        std::string temp = callsign.substr(lastNumber + 1, callsign.length()); 
        if (temp != "quebec" || temp != "charlie") { 
            callsignNumber.push_back(temp); 
            callsign = callsign.substr(0, lastNumber); 
        } 
    } 
    // Convert the call sign word number to an integer and store it in the command struct 
    qcCommand.copter = convertStringToNumber(callsignNumber); 
  
    // Convert the command into an integer 
    std::string command = hyp.substr(commandStart, hyp.length()); 
  
    // TODO: Hardcoded for right now, but later on do it in such a way as to allow for growth of commands. 
    int comResult; 
      
    if (command == "hover") 
        comResult = 0; 
    else if (command == "stop") 
        comResult = 1; 
    else if (command == "move left") 
        comResult = 2; 
    else if (command == "move right") 
        comResult = 3; 
    else if (command == "move forward" || command == "move forwards") 
        comResult = 4; 
    else if (command == "move backwards" || command == "move backward") 
        comResult = 5; 
    else if (command == "take off") 
        comResult = 6; 
    else if (command == "square" || command == "square pattern") 
        comResult = 7; 
    else if (command == "scan" || command == "scan pattern") 
        comResult = 8; 
    else if (command == "figure eight" || command == "figure eight pattern") 
        comResult = 9; 
    else{ 
        comResult = 0;                  //for debugging purposes.  
        return false;                   // There was an error with the command. 
    } 
    // Store the command result in the command struct 
    qcCommand.command = comResult; 
  
    return true;                        // Conversion was successful. 
  
      
} 
  
/** Converts a word number to an int number. 
* 
*   @param number       The number to convert in little endian mode (Least significant digit is at position 0). 
*   @return             The number as an integer. These numbers are positive. If negative, then there is an error. 
*/
int VoiceCommand::convertStringToNumber(std::vector<std::string> number) { 
    int result = 0,                 // The converted number 
        multiplier = 1;             // Points to the current digit position. 
    // Handle the first position 
    try { 
        int temp = numberWords.at(number[0]); 
        if ( temp < 10) 
            result += temp; 
        else { 
            result += temp; 
            multiplier *= 10; 
        } 
  
    } catch (std::out_of_range &e) { 
        std::cerr << "Number does not follow correct format" << std::endl; 
        return -1; 
    } 
    for(int i = 1; i < number.size(); i++) { 
        try { 
            int temp = numberWords.at(number[i]); 
            if (temp < 10) { 
                multiplier *= 10;           // Increase to then next digit position (ex. from tens to hundreds). 
                result += (temp * multiplier);      // Add the single digit to the result. 
            } 
            else { 
                result += (temp * multiplier);                 // Add a double digit number to the result 
                multiplier *= 10;                              // Raise multiplier to match new digit position 
            } 
        } catch (std::out_of_range &e) { 
            // This is in the case of "thousand", "and", "hundred". Just ignore them. 
            // Have to do it since the hash map throws an exception when a key is not found. 
        } 
    } 
    return result; 
} 
  
/** Loads the words for numbers from the numbers list 
*/
void VoiceCommand::loadNumberWords() { 
    std::ifstream list("../lists/numbers.dat"); 
  
    while (!list.eof()) { 
        std::string stringTemp; 
        int numberTemp; 
        list >> stringTemp; 
        list >> numberTemp; 
  
        numberWords.emplace(stringTemp, numberTemp); 
    } 
          
} 
  
/** Send the command over the established connection. 
* 
*   @return         True if the command was successfully sent. 
*/
bool VoiceCommand::sendCommand() 
{ 
    char buffer[2]; 
    buffer[0] = qcCommand.copter + 48; 
    buffer[1] = qcCommand.command + 48; 
    //buffer[2] = '\0'; 
  
    int sendResult = send(*socket, buffer, strlen(buffer), 0); 
  
    if (sendResult != SOCKET_ERROR) 
        return true; 
    else
        return false; 
}