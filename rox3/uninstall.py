import os
import sys
import string

#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('uninstall.py [interface 1] [interface 2] ... [interface N]\n')

#main function
path='/home/wei/rox3'
device='/dev/rox'

if len(sys.argv)>=2:
	#Clear Linux Traffic Control configuration in each interface
	for i in range(1,len(sys.argv)):
		#print sys.argv[i]+'\n'
		os.system('python tc.py clear '+sys.argv[i]+'\n') 
	#Uninstall RoX kernel module
	os.system('rmmod rox\n')     
	#Remove character device 
	os.system('rm '+device+'\n') 
	#Kill all farpd processes
	os.system('killall -HUP farpd\n') 
	#Kill all iperf processes
	os.system('killall -HUP -9 iperf\n') 
	#Kill end_host.py
	os.system('killall -HUP -9 python\n') 
else:
	usage()