#include <stdio.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


//error function
void error(const char *msg) { perror(msg); exit(1); } 
void bind_(struct sockaddr_in * ADDRESS, int * SOCKET);
void receive_conn(char buffer[], int * CONNECTION);
void setup(int * SOCKET, int * option);
int countFiles(DIR * dir, struct dirent * file_in);
void getFiles(DIR * dir, struct dirent * file_in, char * buffer[]);
void sendFile(char * filename, int DATA_CONNECTION);
void sendFileName(char * filename, int DATA_CONNECTION);
void con(struct sockaddr_in * ADDRESS, int * PORT);
void accept_conn(struct sockaddr_in * ADDRESS, int * SOCKET, int * CONNECTION);
void parse_cmd(char buffer[], char * command, char * file, char * sbPort);


//main entry point
int main(int argc, char *argv[])
{
	//declare sockets, buffers
	int RECVSOCKET, CONTROL_CONNECTION, DATA_CONNECTION, DATA_PORT, CONTROL_PORT; //CONTROL_PORT is for extra credit
	int option = 1;
	char buffer[4096];
	struct sockaddr_in SERVER_ADDRESS, CLIENT_ADDRESS;
	
	CONTROL_PORT = atoi(argv[1]); // Get the port number, convert to an integer from a string
	
	con(&SERVER_ADDRESS, &CONTROL_PORT);
 
	setup(&RECVSOCKET, &option);
	
	// Enable the socket to begin listening
{
	// Connect socket to port
	if (bind(RECVSOCKET, (struct sockaddr *)&SERVER_ADDRESS, sizeof(SERVER_ADDRESS)) < 0) 	
	{  
		error("SERVER: ERROR on binding address to socket");
	}
	listen(RECVSOCKET, 5); // Flip the socket on - it can now receive up to 5 connections

	accept_conn(&CLIENT_ADDRESS, &RECVSOCKET, &CONTROL_CONNECTION);

	receive_conn(buffer, &CONTROL_CONNECTION);

	char * command; char * file; char * sbPort;
//	parse_cmd(buffer, command, file, sbPort);
	
	//tokenize the command, filename (if necessary), and the data port number
  	command = strtok(buffer, "@"); 			
  	file = strtok(NULL, "@");
	sbPort = strtok(NULL, " "); 		

	//print what the client sent
	printf("message: %s\n", buffer);
	printf("the command was: %s\n", command);
	printf("the file name was: %s\n", file);
	printf("the sendBackPort was: %s\n", sbPort); //currently string, change to int with atoi when using

	
	//try to print out the client info (addr) to see if you can send back on it
	//currently string, change to int with atoi when using
	//printf("server IP address is: %s\n", inet_ntoa(SERVER_ADDRESS.sin_addr));
	printf("client IP address is: %s\n", inet_ntoa(CLIENT_ADDRESS.sin_addr));
	
	//set up the address struct for the send back to client
	//memset((char*)&CLIENT_ADDRESS, '\0', sizeof(CLIENT_ADDRESS)); //dont think I need this
	DATA_PORT = atoi(sbPort); 				    						//may need to change to int first
	CLIENT_ADDRESS.sin_family = AF_INET; 						    //create a network-capable socket
	CLIENT_ADDRESS.sin_port = htons(DATA_PORT); 		      	    //store the port number


	//convert the machine name into a special addr (dont think I need this)
	//char hostname[256];
	//serverHostInfo = gethostbyname(argv[1]); 
	//if (serverHostInfo == NULL) { error("CLIENT: ERROR no hostname in cmd line arg"); }

	
	//copy in the adddress (dont think I need this)
	//memcpy((char*)&CLIENT_ADDRESS.sin_addr.s_addr, (char*)->h_addr, serverHostInfo->h_length); 


	//create the socket
	DATA_CONNECTION = socket(AF_INET, SOCK_STREAM, 0); 
	if (DATA_CONNECTION < 0) 
	{
		error("CLIENT: ERROR opening socket");
	}
	setsockopt(DATA_CONNECTION, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

	
	//connect socket to addr
	if (connect(DATA_CONNECTION, (struct sockaddr*)&CLIENT_ADDRESS, sizeof(CLIENT_ADDRESS)) < 0) 
	{	
		error("CLIENT: ERROR connecting"); 
	}
	
	if(strcmp(command, "-g") == 0) 
	{
		sendFile(file, DATA_CONNECTION);
	}		
	else if(strcmp(command, "-l") == 0) 
	{
		//count number of files in dir


		//send over that number
		//wait for acknowledgment
		//send file names
		//have server sned you the filename it wants		


		DIR * dir = opendir(".");
		struct dirent * file_in;	
		if (dir == NULL) { error("Could not open current directory" ); exit(1);}
		int numfiles = countFiles(dir, file_in);
		printf("numfiles: %d\n", numfiles);
		char * buffer[numfiles];
		getFiles(dir, file_in, buffer);
		int x = 0;
		for(x; x < numfiles; x++)
		{
			char sbuff[4096];
			memset(sbuff, '\0', sizeof(sbuff));	
			strcpy(sbuff, buffer[x]);
			strcat(sbuff, "#");
			//if(x == numfiles-1) strcat(sbuff, "%");
			printf("filename: %s\n", buffer[x]);
			int chars = 0;
			while(chars < strlen(buffer[x]))
			{ chars += send(DATA_CONNECTION, sbuff, strlen(sbuff), 0); }	
		}
		x = 0;
		for(x; x < numfiles; x++)
		{
			sendFile(buffer[x], DATA_CONNECTION);
		}

	}		
	else
	{error("bad command: ");}
	

	close(CONTROL_CONNECTION); 
	close(RECVSOCKET); //close control port socket
	close(DATA_CONNECTION);   //close data port socket
	return 0; 
	}
}
void setup(int * SOCKET, int * option)
{	
	// Set up the socket
	*SOCKET = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (*SOCKET < 0) error("SERVER: ERROR opening socket");
	setsockopt(*SOCKET, SOL_SOCKET, SO_REUSEADDR, option, sizeof(int)); //from piazza
}

void con(struct sockaddr_in * ADDRESS, int * PORT)
{
	//set up the address struct for this process (the serve
	memset((char *)ADDRESS, '\0', sizeof(ADDRESS)); // Clear out the address struct
	ADDRESS->sin_family = AF_INET; // Create a network-capable socket
	ADDRESS->sin_port = htons(*PORT); // Store the port number
	ADDRESS->sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
}


int countFiles(DIR * dir, struct dirent * file_in)
{
	int fileNum = 0;
	while ((file_in = readdir(dir)) != NULL)
	{ 
		if(strcmp(file_in->d_name, ".") != 0 && strcmp(file_in->d_name, "..") != 0)
		{
			if(strstr(file_in->d_name, ".txt") != NULL) 
			{
				fileNum++;
				
			}
		}
	}
	rewinddir(dir);
	return fileNum;
}

void getFiles(DIR * dir, struct dirent * file_in, char * buffer[])
{
	
	int fileNum = 0;
	while ((file_in = readdir(dir)) != NULL)
	{ 
		if(strcmp(file_in->d_name, ".") != 0 && strcmp(file_in->d_name, "..") != 0)
		{
			if(strstr(file_in->d_name, ".txt") != NULL) 
			{
				buffer[fileNum] = (char *)malloc(strlen(file_in->d_name));
				strcpy(buffer[fileNum], file_in->d_name);
				//printf("buffer: %s\n", buffer[fileNum]);
				fileNum++;	
			}
		}
	}
	closedir(dir);
}


void sendFileName(char * filename, int DATA_CONNECTION)
{
	int chars = 0;
	printf("%s\n", filename); 
	//send over filenames for now (file contents later)
	while(chars < strlen(filename)) 
	{
		chars += send(DATA_CONNECTION, filename, strlen(filename), 0); 
		printf("chars sent: %d\n", chars);
	}
}

void sendFile(char * filename, int DATA_CONNECTION)
{
		FILE * fp;
		fp = fopen(filename,"r");
		char buffer[4096];
		memset(buffer, '\0', sizeof(buffer)); 
		
		//seek to the beginning of the file
		fseek(fp, 0, SEEK_SET);
		
		//read contents of file
		fgets(buffer, sizeof(buffer), fp);	
		
		printf("file contents: %s\n", buffer);		

		//send it 
		int chars = 0;
		while(chars < strlen(buffer)) 
		{
			chars += send(DATA_CONNECTION, buffer, strlen(buffer), 0); 
			printf("chars sent: %d\n", chars);
		}

		fclose(fp);
}

//void bind_(struct sockaddr_in * ADDRESS, int * SOCKET)
void accept_conn(struct sockaddr_in * ADDRESS, int * SOCKET, int * CONNECTION)
{

	socklen_t sizeOfClientInfo;
	//accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(*ADDRESS); //get the size of the address for the client that will connect
	*CONNECTION = accept(*SOCKET, (struct sockaddr *)ADDRESS, &sizeOfClientInfo); 
	if (*CONNECTION < 0) { error("SERVER: ERROR on accept! exit "); }
}


void receive_conn(char buffer[], int * CONNECTION)
{
	//make sure buffer is null terminated
	memset(buffer, '\0', 4096);
	
	//receive data from client
	int charsRead;
	charsRead = recv(*CONNECTION, buffer, 4096, 0);
	printf("chars read: %d\n", charsRead);

}

void parse_cmd(char buffer[], char * command, char * file, char * sbPort)
{
}
