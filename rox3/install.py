import os
import sys
import string

#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('install.py init\n')
	sys.stderr.write('install.py add [interface] [IP address] [path ID 1] [path ID 2] ... [path ID N]\n')

#main function
#global variables
path='/home/wei/rox3'
device='/dev/rox'
major_num=100

if len(sys.argv)==2: 			
	#install RoX 
	# 1. Compile and insert RoX kernel module
	# 2. Compile user-space control program
	# 3. Install virtual character device (rox, major number: 100)
	# 4. Start end_host.py
	if sys.argv[1]=='init':
		#Compile rox kernel module
		os.system('make\n')   
		#Compule user-space control program (a.out)
		os.system('gcc ioctl.c\n')  
		#Install character device 
		os.system('mknod '+device+' c '+str(major_num)+' 1\n')  
		#Insert kernel module
		os.system('insmod rox.ko\n') 
		#Start communication script
		os.system('nohup python end_host.py &') 
	else:
		usage()
		
elif len(sys.argv)>=5:        
	#add NIC and related path IDs
	# 1. Add NAT rules
	# 2. Start farpd on specific interfaces
	if sys.argv[1]=='add':
		#Initialize Linux Traffic Control on specific interface
		os.system('python tc.py init '+sys.argv[2]+'\n')   
		cmd='farpd -i '+sys.argv[2]+' '+sys.argv[3]+' '
		for i in range(4,len(sys.argv)):
			cmd=cmd+sys.argv[i]+' '
			#Add first-class rule
			os.system('./a.out insert 1 0.0.0.0 '+sys.argv[i]+' 0 0 0 '+sys.argv[3]+' 0 0\n') 
		cmd=cmd+'\n'
		#Start farpd
		os.system(cmd) 
	else:
		usage()
else:
	usage()