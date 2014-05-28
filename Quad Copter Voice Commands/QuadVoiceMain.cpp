#include <WinSock2.h> 
#include <iostream> 
#include <string> 
#include "VoiceCommand.h" 
  
//Function Prototypes 
SOCKET awaitConnection(char*); 
SOCKET connectToController(char*, char*); 
SOCKET createSocket(); 
  
int main(int argc, char* argv[]) { 
    // Ensure that the IP address and port were passed as command-line arguments 
    if (argc != 3) { 
        std::cerr << "Missing IP address and/or port." 
            << "\nTo connect to an address use: Voice Command.exe [IP Address] [Port Number]"
            << "\nTo wait for a connection use: Voice Command.exe -ls [Port Number]" << std::endl; 
        return 1; 
    } 
  
    // Get the socket that will be used to send commands to controller 
    SOCKET commandSocket = NULL; 
    if (strcmp(argv[1], "-ls") == 0) { 
        std::cout << "Awaiting connection at port " << argv[2] << std::endl; 
        commandSocket = awaitConnection(argv[2]); 
    } 
    else { 
        std::cout << "Establishing connection to " << argv[1] << std::endl; 
        commandSocket = connectToController(argv[1], argv[2]); 
    } 
  
    if (commandSocket == NULL) { 
        std::cerr << "Error in establishing the connection." << std::endl; 
        return 1; 
    } 
  
    // Disable the output of keys to the console 
    // from: http://stackoverflow.com/questions/1413445/read-a-password-from-stdcin 
    HANDLE stdinHandle = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD mode; 
    GetConsoleMode(stdinHandle, &mode); 
    mode &= ~ENABLE_ECHO_INPUT; 
    SetConsoleMode(stdinHandle, mode); 
  
    // Begin the actual voice command program 
    try { 
        VoiceCommand voxCommand(&commandSocket, argv[0]); 
        voxCommand.startVoiceCommand(); 
    } 
    catch (ConfigNullException &e) { 
        //There was an error with opening the models and dictionary. 
        std::cerr << e.what() << std::endl; 
    } catch (DecoderNullException &e) { 
        //There was an error with creating the decoder 
        std::cerr << e.what() << std::endl; 
    } 
  
    // Close down socket and clean up connection 
    closesocket(commandSocket); 
    WSACleanup(); 
  
    return 0; 
} 
  
/** Creats a listen socket at the dictated port, and it awaits for a connection. 
* 
*   @param port             The port to listen on for a connection. 
*   @return                 Returns the socket if connection was successful, else it returns NULL. 
*/
SOCKET awaitConnection(char* port) { 
    // Get the uninitalized socket 
    SOCKET socket = createSocket(); 
  
    sockaddr_in address; 
    address.sin_family = AF_INET;                           // Sets socket type, IPv4 
    address.sin_addr.S_un.S_addr = INADDR_ANY;              //Attaches the socket to all network interfaces. 
    address.sin_port = htons((short)(atoi(port)));          // Port to connect to. 
  
    // Ensure the socket binded correctly 
    if (bind(socket, (sockaddr*)&address, sizeof address) == SOCKET_ERROR) { 
        std::cout <<"ERROR binding at" << port << std::endl; 
        return NULL; 
    } 
  
    // Wait for a connection 
    while (listen(socket, SOMAXCONN) == SOCKET_ERROR); 
  
    SOCKET client;                      // Client that is connecting 
    int length = sizeof address; 
  
    client = accept(socket,(sockaddr*) &address, &length); 
  
    // Ensure that the connection was successful 
    if (client == INVALID_SOCKET) 
        return NULL; 
  
    // Send and ACK to the client. 
    send(client, "##", 2, 0); 
  
    return client;                      // Connection successful, return the socket. 
} 
  
/** Connects the program to the IP address and port passed through the command-line. It handles all of the errors 
* that may occur. 
* 
* @param ipAddress          The IP address of the controller 
* @param port               The port to connect to at the given IP address 
* @return                   Returns the socket if connection was successful, else it returns NULL 
**/
SOCKET connectToController(char* ipAddress, char* port) { 
    // Get an uninitialized socket 
    SOCKET socket = createSocket(); 
  
    // Add address info to the socket. 
    sockaddr_in address; 
    address.sin_family = AF_INET;                           // Sets the socket type, IPv4 
    address.sin_addr.S_un.S_addr = inet_addr(ipAddress);    // Address that socket will connect to 
    u_short prt = u_short(atoi(port));                       
    address.sin_port = htons(prt);                          // Port to connect to. 
  
    // Setup Complete. Connect to the controller 
    int connectionResult = connect(socket, (sockaddr*)&address, sizeof(address)); 
    // Check for an error when connecting. If there is one, then close the socket 
    if (connectionResult == SOCKET_ERROR) { 
        std::cout << "Error when attempting connection." << std::endl; 
        closesocket(socket); 
        WSACleanup(); 
        return NULL; 
    } 
  
    // Connection was succesfully established 
    return socket; 
} 
  
/** Creates an uninitialized socket, and checks the version of the current Ws2_32.dll. This version must be at least 2.2. 
* 
*   @return             The uninitialized socket. 
*/
SOCKET createSocket() { 
    WSAData version;                                    // Used to determine the version of the Ws2_32.dll. Needs to be at least version 2.2 
    WORD vers = MAKEWORD(2,2);                          // Make a WORD version of 2.2 so it can be compared with "version" 
      
    int versionCompare = WSAStartup(vers, &version);    // Find out if the Ws2_32.dll is the correct version 
    if (versionCompare != 0) {                          // The version is not at least 2.2 
        // Print error message and what caused it. 
        std::cout << "The version of Ws2_32.dll is not at least 2.2 -\n" << WSAGetLastError() << std::endl; 
        return NULL; 
    } 
  
    // Create the socket. AF_INET = IPv4, SOCK_STREAM = TCP byte stream, IPPROTO_TCP = using TCP 
    SOCKET result = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); 
    // Check that socket was opened correctly 
    if (result == INVALID_SOCKET) { 
        std::cout << "Error creating socket." << std::endl; 
        return NULL; 
    } 
  
    return result; 
}