import os
import socket
import sys

'''
This should receive one file from the sever

'''

#create a socket
SENDSOCK = socket.socket()

#get ip address of server from cmd line
SERVER_HOST = str(sys.argv[1])

#get control port from cmd line
CONTROL_PORT = int(sys.argv[2])

#get cmd from cmd line
CMD = str(sys.argv[3])

#get data port from cmd line
DATA_PORT = int(sys.argv[5])

#connect socket to server
SENDSOCK.connect((SERVER_HOST, CONTROL_PORT))


#get the filename
FILENAME = str(sys.argv[4])

#gather msg to send to server
SEND_MSG = "@" + CMD + "@" + FILENAME + "@" + str(DATA_PORT)

print SEND_MSG

#close control socket?

#make new socket
RECVSOCK = socket.socket()

#bind to server as if you were the server
RECVSOCK.bind(("127.0.0.1", DATA_PORT)) #change to server host?
print ">> SERVER AWAKE!"

#send command and the data port over to server
SENDSOCK.send(SEND_MSG)

#receive stuff back
if CMD == "-g":
#while(1):
	RECVSOCK.listen(1)
	conn, addr = RECVSOCK.accept()  
	rec_msg = conn.recv(4096).strip()
	with open(FILENAME, 'wb') as file:
		#print what server sent
		print "Receiving \"%s\" \nfrom %s" % (FILENAME, addr) 
		file.write(rec_msg)