import os
import socket
import sys
import re


class client_FTP:
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

	def connectControlPort(self):
		#connect socket to server
		self.SENDSOCK.connect((self.SERVER_HOST, self.CONTROL_PORT))
		
	def connectDataPort(self):
		#bind to server as if you were now the server
		self.RECVSOCK.bind((self.SERVER_HOST, self.DATA_PORT)) #change ip # to server host?
		print ">> Ready for data on data port!" 
		
	def sendFileRequest(self, filename):
		#gather msg to send to server
		SEND_MSG = "@" + self.CMD + "@" + filename + "@" + str(self.DATA_PORT)

		#print msg that is being sent to console
		print SEND_MSG

		#send command and the data port over to server
		self.SENDSOCK.send(SEND_MSG)
	
	def accept(self):	
		self.RECVSOCK.listen(5)
		self.conn, self.addr = self.RECVSOCK.accept()  
		
	def receiveFile(self, filename):
		#receive one file
		rec_msg = self.conn.recv(4096).strip()
		if not rec_msg:
			rec_msg = self.conn.recv(4096).strip()

		#if not rec_msg: break
		with open(filename, 'wb') as file:
			#print filename and server address
			print "Receiving \"%s\" \nfrom %s" % (filename, self.addr)
			#write contents to file
			file.write(rec_msg)
				
	def receiveDir(self):
		lis = []
		rec_msg = self.conn.recv(4096).strip()
		while rec_msg:
			while rec_msg.find('#') != -1:
				name, rec_msg = rec_msg.split('#', 1)
				lis.append(name)	
		for t in lis:
			self.receiveFile(t)
	
#main entry point
if __name__ == "__main__":
	ftp = client_FTP(str(sys.argv[1]), int(sys.argv[2]), str(sys.argv[3]), str(sys.argv[4]), int(sys.argv[5]))
	ftp.connectControlPort()
	ftp.connectDataPort()
	ftp.sendFileRequest(ftp.FILENAME)
	ftp.accept()
	if ftp.CMD == "-g":
		ftp.receiveFile(ftp.FILENAME)
	elif ftp.CMD == "-l":
		ftp.receiveDir()
	
