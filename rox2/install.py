import os
import sys
import string

#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('install.py [interface] [IP address] [path ID 1] [path ID 2] ... [path ID N]\n')

#main function
path='/home/wei/rox'
device='/dev/rox'
major_num=100
#
if len(sys.argv)>=4: 			#At least one path id
	os.system('cd '+path+'\n')  #Come to source code directory
	os.system('make\n')         #Compile rox kernel module
	os.system('gcc ioctl.c\n')  #Compule user-space control program (a.out)
	os.system('mknod '+device+' c '+str(major_num)+' 1\n')  #Install character device 
	os.system('insmod rox.ko\n') #Insert kernel module
	os.system('python tc.py init '+sys.argv[1]+'\n')   #Initialize Linux Traffic Control on specific interface
	cmd='farpd -i '+sys.argv[1]+' '+sys.argv[2]+' '
	for i in range(3,len(sys.argv)):
		cmd=cmd+sys.argv[i]+' '
		os.system('./a.out insert 1 0.0.0.0 '+sys.argv[i]+' 0 0 0 '+sys.argv[2]+' 0 0\n') #Add first-class rule
	cmd=cmd+'\n'
	os.system(cmd) #Start farpd
	os.system('nohup python end_host.py &') #Start communication script
else:
	usage()