/*******************************
* Name: Robert Boespflug
* Date: 06/05/2018
* Description: This is the file transfer server. This program 
* waits for a cmd from the ftclient and sends either the file that
* was requested or the entire current directory. Txt files only. 
********************************/


#include <stdio.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>


//error function calls perror and exits
void error(const char *msg) { perror(msg); exit(1); }

//function prototypes 
void bind_(struct sockaddr_in * ADDRESS, int * SOCKET);
void receive_conn(char buffer[], int * CONNECTION);
void setup_sock(int * SOCKET, int * option);
int countFiles(DIR * dir, struct dirent * file_in);
void getFiles(DIR * dir, struct dirent * file_in, char * buffer[]);
void sendFile(char * filename, int DATA_CONNECTION);
//void sendFileName(char * filename, int DATA_CONNECTION);
void connect_conn(struct sockaddr_in * ADDRESS, int * PORT);
void accept_conn(struct sockaddr_in * ADDRESS, int * SOCKET, int * CONNECTION);
//void parse_cmd(char buffer[], char * command, char * file, char * sbPort);


//main entry point
int main(int argc, char *argv[])
{
	//declare sockets, buffers
	int RECVSOCKET, CONTROL_CONNECTION, DATA_CONNECTION, DATA_PORT, CONTROL_PORT;
	int option = 1;
	char buffer[4096];
	struct sockaddr_in SERVER_ADDRESS, CLIENT_ADDRESS;
	
	//get the port number from cmd line, convert to an integer from a string
	CONTROL_PORT = atoi(argv[1]); 
	
	//set up the connection to client
	connect_conn(&SERVER_ADDRESS, &CONTROL_PORT); 
	setup_sock(&RECVSOCKET, &option);
	
	//bind socket to port to create server
	if (bind(RECVSOCKET, (struct sockaddr *)&SERVER_ADDRESS, sizeof(SERVER_ADDRESS)) < 0) 	
	{  
		error("SERVER: ERROR on binding address to socket");
	}
	while(1)
	{
		//listen for incoming connections, up to five
		listen(RECVSOCKET, 5); 

		//accpet a connection and receive command from client
		accept_conn(&CLIENT_ADDRESS, &RECVSOCKET, &CONTROL_CONNECTION);
		receive_conn(buffer, &CONTROL_CONNECTION);

		//tokenize the command, filename (if necessary), and the data port number
		char * command; char * file; char * sbPort;
		command = strtok(buffer, "@"); 			
		if(strcmp(command, "-g") == 0) file = strtok(NULL, "@");
		if(strcmp(command, "-l") == 0) sbPort = strtok(NULL, "@");
		else sbPort = strtok(NULL, " "); 	

		//print what the client sent
		/*printf("message: %s\n", buffer);
		printf("the command was: %s\n", command);
		printf("the file name was: %s\n", file);
		printf("the sendBackPort was: %s\n", sbPort);*/

		//data to screen for info
		/*printf("client IP address is: %s\n", inet_ntoa(CLIENT_ADDRESS.sin_addr));*/
		
		//set up the address struct for the send-back to client
		DATA_PORT = atoi(sbPort); 				    					//change to int first
		CLIENT_ADDRESS.sin_family = AF_INET; 						    //create a network socket
		CLIENT_ADDRESS.sin_port = htons(DATA_PORT); 		      	    //store the port number in network order


		//create the socket and set options to be able to quickly reuse the socket
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
		
		//depending on which cmd the client sent, send back
		//either the file name requested or the entire contents 
		//of the current working dir 
		if(strcmp(command, "-g") == 0) 
		{
			sendFile(file, DATA_CONNECTION);
		}		
		else if(strcmp(command, "-l") == 0) 
		{
			//open current dir and init dirent to hold for dir properties
			DIR * dir = opendir(".");
			if(dir == NULL) error("Could not open current directory" );
			struct dirent * file_in;

			//count the number of txt files in the current dir
			int numfiles = countFiles(dir, file_in);
			printf("number of txt files: %d\n", numfiles);
			
			//declare an array of strings that represents the filenames
			char * buffer[numfiles];
			
			//the getFiles function will put all the .txt filenames
			//in the buffer of strings
			getFiles(dir, file_in, buffer);
			
			//for every file in the filename array
			int x = 0;
			for(x; x < numfiles; x++)
			{
				//make a send buffer to gather the filenames and
				//delimiters into one string to send over to the client
				char sbuff[4096];
				memset(sbuff, '\0', sizeof(sbuff));	
				strcpy(sbuff, buffer[x]);
				strcat(sbuff, "#");
				
				//send over the send buffer
				int chars = 0;
				while(chars < strlen(buffer[x]))
				{ chars += send(DATA_CONNECTION, sbuff, strlen(sbuff), 0); }	
			}
			
			//for every file in the filename buffer, 
			//send over that filename
			x = 0;
			for(x; x < numfiles; x++)
			{
				sendFile(buffer[x], DATA_CONNECTION);
			}

		}		
		else
		{error("bad command: ");}
		

		//close(CONTROL_CONNECTION); 	//close the connection to client
		//close(RECVSOCKET); 		  	//close control port socket
		close(DATA_CONNECTION);   	//close data port socket
	}
	return 0; 

}

/*************************
* Description:
*************************/
void setup_sock(int * SOCKET, int * option)
{	
	// Set up the socket
	*SOCKET = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (*SOCKET < 0) error("SERVER: ERROR opening socket");
	setsockopt(*SOCKET, SOL_SOCKET, SO_REUSEADDR, option, sizeof(int)); //from piazza
}

/*************************
* Description:
*************************/
void connect_conn(struct sockaddr_in * ADDRESS, int * PORT)
{
	//set up the address struct for this process (the serve
	memset((char *)ADDRESS, '\0', sizeof(ADDRESS)); // Clear out the address struct
	ADDRESS->sin_family = AF_INET; // Create a network-capable socket
	ADDRESS->sin_port = htons(*PORT); // Store the port number
	ADDRESS->sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process
}

/*************************
* Description:
*************************/
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

/*************************
* Description:
*************************/
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




/*************************
* Description:
*************************/
void sendFile(char * filename, int DATA_CONNECTION)
{
		//try to open the passed in filename
		printf("Sending over: %s\n", filename);
		FILE * fp;
		fp = fopen(filename,"r");
		
		//if no filename then send back error and stop
		if(!fp) {
			send(DATA_CONNECTION, "error", strlen("error"), 0);
			error("No file found!");
		}
		
		//else file does exist, so make a buffer to hold
		//the files contents to send back
		char buffer[4096];
		memset(buffer, '\0', sizeof(buffer)); 
		
		//seek to the beginning of the file
		fseek(fp, 0, SEEK_SET);
		
		//read contents of file into buffer
		fgets(buffer, sizeof(buffer), fp);	
		
		//print file contents to window
		//printf("file contents: %s", buffer);		

		//send the contents of the file client and close file
		int chars = 0;
		while(chars < strlen(buffer)) 
		{
			chars += send(DATA_CONNECTION, buffer, strlen(buffer), 0); 
			printf("chars sent: %d\n", chars);
		}
		fclose(fp);
}

/*************************
* Description:
*************************/
void accept_conn(struct sockaddr_in * ADDRESS, int * SOCKET, int * CONNECTION)
{

	socklen_t sizeOfClientInfo;
	//accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(*ADDRESS); //get the size of the address for the client that will connect
	*CONNECTION = accept(*SOCKET, (struct sockaddr *)ADDRESS, &sizeOfClientInfo); 
	if (*CONNECTION < 0) { error("SERVER: ERROR on accept! exit "); }
}

/*************************
* Description:
*************************/
void receive_conn(char buffer[], int * CONNECTION)
{
	//make sure buffer is null terminated
	memset(buffer, '\0', 4096);
	
	//receive data from client
	int charsRead;
	charsRead = recv(*CONNECTION, buffer, 4096, 0);
	printf("chars read: %d\n", charsRead);

}

