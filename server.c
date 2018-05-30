#include <stdio.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


//this should receive one messgage from the client and print it


//error function
void error(const char *msg) { perror(msg); exit(1); } 


//main entry point
int main(int argc, char *argv[])
{
	//declare sockets, buffers
	int listenSocketFD, establishedConnFD, dataSocketFD, portNumber, c_pid;
	int option = 1;
	socklen_t sizeOfClientInfo;
	char buffer[4096];
	struct sockaddr_in serverAddress, clientAddress;

	//set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process


	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("SERVER: ERROR opening socket");
	setsockopt(listenSocketFD, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	// Enable the socket to begin listening
	// Connect socket to port
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) 	
	{  
		error("SERVER: ERROR on binding");
	}
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections


	//accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); //get the size of the address for the client that will connect
	establishedConnFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); 
	if (establishedConnFD < 0) { error("SERVER: ERROR on accept! exit "); }

	//make sure buffer is null terminated
	memset(buffer, '\0', 4096);
	int charsRead;

	//receive data from client
	charsRead = recv(establishedConnFD, buffer, 4096, 0);


	//tokenize the command, filename (if necessary), and the data port number
  	char * command = strtok(buffer, "@"); 			
  	char * file = strtok(NULL, "@");
	char * sbPort = strtok(NULL, " "); 		

	//print what the client sent
	printf("chars read: %d\n", charsRead);
	printf("message: %s\n", buffer);
	printf("the command was: %s\n", command);
	printf("the file name was: %s\n", file);
	printf("the sendBackPort was: %s\n", sbPort); //currently string, change to int with atoi when using

	
	//try to print out the client info (addr) to see if you can send back on it
	//currently string, change to int with atoi when using
	//printf("server IP address is: %s\n", inet_ntoa(serverAddress.sin_addr));
	printf("client IP address is: %s\n", inet_ntoa(clientAddress.sin_addr));
	
	//set up the address struct for the send back to client
	//memset((char*)&clientAddress, '\0', sizeof(clientAddress)); //dont think I need this
	portNumber = atoi(sbPort); 				    						//may need to change to int first
	clientAddress.sin_family = AF_INET; 						    //create a network-capable socket
	clientAddress.sin_port = htons(portNumber); 		      	    //store the port number


	//convert the machine name into a special addr (dont think I need this)
	//char hostname[256];
	//serverHostInfo = gethostbyname(argv[1]); 
	//if (serverHostInfo == NULL) { error("CLIENT: ERROR no hostname in cmd line arg"); }

	
	//copy in the adddress (dont think I need this)
	//memcpy((char*)&clientAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); 


	//create the socket
	dataSocketFD = socket(AF_INET, SOCK_STREAM, 0); 
	if (dataSocketFD < 0) 
	{
		error("CLIENT: ERROR opening socket");
	}
	setsockopt(dataSocketFD, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	
	//connect socket to addr
	if (connect(dataSocketFD, (struct sockaddr*)&clientAddress, sizeof(clientAddress)) < 0) 
	{	
		error("CLIENT: ERROR connecting"); 
	}
	
	//check to see if you connect back to client
	send(dataSocketFD, "hello", 5, 0); //send five chars "hello" though data socket
	
	/*
	if(strcmp(command, "-g") == 0) 
	{
		printf("You want file %s on port %d!\n", file, atoi(sbPort));
	}		
	else if(strcmp(command, "-l") == 0) 
	{
		printf("You want the dir!\n");
		DIR * dir = opendir(".");
		struct dirent * file_in;	
		if (dir == NULL) { error("Could not open current directory" ); exit(1);}	
		while ((file_in = readdir(dir)) != NULL)
        { 
		//	if(strcmp(file_in->d_name, ".") != 0 || strcmp(file_in->d_name, "..") != 0)
			//{
				printf("%s\n", file_in->d_name); 
				//make a sendbuffer to send over
				char sendBuffer[4096];
				memset(&sendBuffer, '\0', sizeof(sendBuffer));
				strcat(sendBuffer, file_in->d_name);
				send(listenSocketFD, sendBuffer, strlen(sendBuffer), 0); 
			//}
		}
		//put filenames into variables 
		//sendwith send() on port sbPort
		closedir(dir);
	}		
	else
	{error("bad command: ");}
	*/

	close(establishedConnFD); 
	close(listenSocketFD); // Close the listening socket
	close(dataSocketFD); 
	return 0; 
}
