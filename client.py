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



#close other socket?

#make new socket
RECVSOCK = socket.socket()

#bind to server as if you were the server
RECVSOCK.bind(("127.0.0.1", 29998))
print ">> SERVER AWAKE!"

#send command and the data port over to server
SENDSOCK.send(SEND_MSG)

while(1):
	RECVSOCK.listen(1)
	conn, addr = RECVSOCK.accept()  
	msg = conn.recv(500).strip()
	print msg

'''
with open('rec_file', 'wb') as file:
	#receive stuff back
	rec_msg = SENDSOCK.recv(4096).strip

	#print what server sent
	print "Receiving %s from %s" % (rec_msg, addr) 
	file.write(rec_msg)
'''
