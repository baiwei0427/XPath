import os
import sys
import string

#This function is to print usage of this script
def usage():
	sys.stderr.write('Usage of this script:\n')
	sys.stderr.write('uninstall.py [interface]\n')

#main function
path='/home/wei/rox'
device='/dev/rox'

if len(sys.argv)==2:
	os.system('cd '+path+'\n')   #Come to source code directory
	os.system('rmmod rox\n')     #Uninstall RoX kernel module
	os.system('rm '+device+'\n') #Remove character device 
	os.system('python tc.py clear '+sys.argv[1]+'\n') #Clear Linux Traffic Control configuration in specific interface
	os.system('killall -HUP farpd\n') #Kill all farpd processes
	os.system('killall -HUP -9 iperf\n') #Kill all iperf processes
	os.system('killall -HUP -9 python\n') #Kill end_host.py
else:
	usage()