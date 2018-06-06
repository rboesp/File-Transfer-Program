'''
* Name: Robert Boespflug
* Date: 06/05/2018
* Description: This is the file transfer client. This program
* sends the ftserver either the file it wants to transfer over 
* or all the files in the working directory of the server. Txt files only.
'''

import os
import socket
import sys

'''
This is the client_ft class. This class contains the functions
necessary to get a file or multiple txt files sent over by the ftserver. 
'''
class client_ft:
	'''init function initializes the variables 
	we will need during the program, creates the sending
	and receiving sockets'''
	def __init__(self, SERVER_HOST, CONTROL_PORT, CMD, FILENAME, DATA_PORT):
		self.SERVER_HOST = SERVER_HOST
		self.CONTROL_PORT = CONTROL_PORT
		self.CMD = CMD
		self.FILENAME = FILENAME
		self.DATA_PORT = DATA_PORT
		self.SENDSOCK = socket.socket()
		self.RECVSOCK = socket.socket()		
		self.conn = ''
		self.addr = ''	

	'''This function connects the client to the server thorugh the control port'''
	def connectControlPort(self):
		#connect socket to server
		self.SENDSOCK.connect((self.SERVER_HOST, self.CONTROL_PORT))
		
	'''This function calls the bind function so the client can start receiving
	data after it sends the inital command'''
	def connectDataPort(self):
		#bind to server as if you were now the server
		self.RECVSOCK.bind((self.SERVER_HOST, self.DATA_PORT))
		print ">> Ready for data on data port!" 
		
	'''This function concaconates the final cmd to send (with send() function)
	over to the server. The cmd will include the file name to send
	back and the port to send back the contents of the file.'''
	def sendFileRequest(self, filename):
		#gather msg to send to server
		if self.CMD == "-g": SEND_MSG = "@" + self.CMD + "@" + filename + "@" + str(self.DATA_PORT)
		else: SEND_MSG = "@" + self.CMD + "@" + str(self.DATA_PORT)

		#print msg that is being sent to console
		print SEND_MSG

		#send command and the data port over to server
		self.SENDSOCK.send(SEND_MSG)
	
	'''This function will listen for incoming file contents (with listen()) from the server
	thorugh the data port. The function will then accept the connection by calling accept()'''
	def accept(self):	
		self.RECVSOCK.listen(5)
		self.conn, self.addr = self.RECVSOCK.accept()  
		
		
	'''This function receives one file from the server on the data port. This function takes
	one argument, a string which represents the name of the file we wish to transfer over. 
	This function calls recv(), opens a file with the filename passed as the argument, and 
	calls the python write() function to write the contents into an open file'''
	def receiveFile(self, filename):
		rec_msg = self.conn.recv(4096)
		if not rec_msg:
			rec_msg = self.conn.recv(4096)
		
		if rec_msg == "error": 
			print "error, no file named %s in server" % filename
		else:
			with open(filename, 'wb') as file:
				#print filename and server address
				print "Receiving \"%s\" from %s" % (filename, self.addr)
				
				#write contents to file
				file.write(rec_msg)
	
	'''This function will receive the entire contents of the server's working directory.
	(Txt files). The function takes no arguments. The function calls recv to get the file names
	of all the txt files in the servers directory. The function then calls receivefile() function
	to accept the server sending the files over one-by-one.'''
	def receiveDir(self):
		lis = []
		rec_msg = self.conn.recv(4096).strip()
		while rec_msg:
			while rec_msg.find('#') != -1:
				name, rec_msg = rec_msg.split('#', 1)
				lis.append(name)	
		for t in lis:
			self.receiveFile(t)
		for t in lis:
			print t

'''This is the main entry point of the program. This function creates the file transfer object
   and initializes the objects data variables with the cmd line args.'''
if __name__ == "__main__":
	if(sys.argv[3] == "-g"):
		ft = client_ft(str(sys.argv[1]), int(sys.argv[2]), str(sys.argv[3]), str(sys.argv[4]), int(sys.argv[5]))
	else:
		ft = client_ft(str(sys.argv[1]), int(sys.argv[2]), str(sys.argv[3]), '', int(sys.argv[4]))
	ft.connectControlPort()
	ft.connectDataPort()
	ft.sendFileRequest(ft.FILENAME)
	ft.accept()
	if ft.CMD == "-g":
		ft.receiveFile(ft.FILENAME)
		print "Transfer complete"
	elif ft.CMD == "-l":
		ft.receiveDir()
		#print "Transfer complete"
	
