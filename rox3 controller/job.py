import MySQLdb
import time
import os
import string
import httplib
import urllib
import urllib2
import thread

				
#This function is to start receiver
# 1. set ACK path ID
# 2. start iperf TCP/UDP server 
def start_receiver(sender,receiver,protocol,port,ackpath,dscp):
	
	#set ACK path ID
	#/home/wei/rox3/a.out insert [2] [receiver ip] [sender ip] [protocol] [port] [0] [ACK path ID] [0] [DSCP]
	cmd=path+'/a.out insert 2 '+receiver+' '+sender+' '+str(protocol)+' '+str(port)+' '+'0'+' '+ackpath+' '+'0'+' '+str(dscp)+':'
	
	#start iperf TCP/UDP server
	#python /home/wei/rox3/server.py [protocol] [port]
	cmd=cmd+'python '+path+'/server.py '+str(protocol)+' '+str(port)
	
	#connect receiver to execute commands
	output=os.popen("python ./controller.py "+receiver+" \'"+cmd+"\'")


#This function is to start sender
# 1. set data path ID
# 2. set tc class to do traffic shaping (rate limiting) 
# 3. start iperf TCP/UDP client
#Compared with start_receiver it calls for three extra parameters: jobid, volume, rate, netdev and classid
# 1. tc class of a job=classid (starting from 2 because htb 1:1 is default class)
# 2. nfmark for data flow of a job=classid 
# 3. netdev identifies interface for tc 
# 4. output result file=/home/wei/result/job_[jobid]
def start_sender(sender,receiver,jobid,protocol,port,netdev,classid,datapath,volume,rate,dscp):

	#set data path ID
	#/home/wei/rox3/a.out insert [2] [sender ip] [receiver ip] [protocol] [0] [port] [data path ID] [classid] [DSCP]
	cmd=path+'/a.out insert 2 '+sender+' '+receiver+' '+str(protocol)+' '+'0'+' '+str(port)+' '+datapath+' '+str(classid)+' '+str(dscp)+':'

	#add tc class 
	#python /home/wei/rox3/tc.py add [netdev] [classid] [rate] [classid]
	cmd=cmd+'python '+path+'/tc.py add '+netdev+' '+str(classid)+' '+str(rate)+' '+str(classid)+':'
	
	#start iperf client
	#python /home/wei/rox3/client.py [receiver ip] [protocol] [port] [volume] [time interval] [output file]
	cmd=cmd+'python '+path+'/client.py '+receiver+' '+str(protocol)+' '+str(port)+' '+str(volume)+' '+str(interval)+' '+output_result+'/job_'+str(jobid)
	
	#connect sender to execute commands
	output=os.popen("python ./controller.py "+sender+" \'"+cmd+"\'")

	
#This function is to change configurations at receiver side
#You can only change ACK path ID and DSCP at receiver side
def update_receiver(sender,receiver,protocol,port,ackpath,dscp):

	#update ACK path ID
	#/home/wei/rox3/a.out update [2] [receiver ip] [sender ip] [protocol] [port] [0] [ACK path ID] [0] [DSCP]
	cmd=path+'/a.out update 2 '+receiver+' '+sender+' '+str(protocol)+' '+str(port)+' '+'0'+' '+ackpath+' '+'0'+' '+str(dscp)
	
	#connect sender to execute commands
	output=os.popen("python ./controller.py "+receiver+" \'"+cmd+"\'")

	
#This function is to change configurations at sender side
#You can change data path ID, rate and DSCP at sender side
def update_sender(sender,receiver,protocol,port,netdev,classid,datapath,rate,dscp):

	#update data path ID
	#/home/wei/rox3/a.out insert [2] [sender ip] [receiver ip] [protocol] [0] [port] [data path ID] [classid] [DSCP]
	cmd=path+'/a.out update 2 '+sender+' '+receiver+' '+str(protocol)+' '+'0'+' '+str(port)+' '+datapath+' '+str(classid)+' '+str(dscp)+':'
	
	#update tc class 
	#python /home/wei/rox3/tc.py change [netdev] [classid] [rate]
	cmd=cmd+'python '+path+'/tc.py change '+netdev+' '+str(classid)+' '+str(rate)
	
	#connect sender to execute commands
	output=os.popen("python ./controller.py "+sender+" \'"+cmd+"\'")

	
try:
	#global variables
	global path,output_result,interval
	
	#Default directory for RoX in each end host
	#For RoX 1.0, it's /home/wei/rox
	#For RoX 1.1, it's /home/wei/rox2
	#For RoX 1.2, it's /home/wei/rox3
	path='/home/wei/rox3'
	
	#Default directory to store output results in each end host
	output_result='/home/wei/result'
	#Default time interval (second) to record instant throughput 
	interval=1

	#Connect database	
	#For RoX 1.2, database is moved from rox to rox3 because table structured has been changed
	conn=MySQLdb.connect(host='localhost',user='root',passwd='root',db='rox3',port=3306)
	cur=conn.cursor()
	
	while(True):
		#Find a new job which is ready to start
		count=cur.execute('select * from job where UNIX_TIMESTAMP(now())>UNIX_TIMESTAMP(starttime) and is_start=0')
		if count>0:
			#Get all SQL query results 
			results=cur.fetchall()
			
			#Table job structure
			#0  jobid int unsigned auto_increment primary key,
			#1  sender varchar(15),
			#2  receiver varchar(15),
			#3  port int unsigned,
			#4  datapath varchar(15),
			#5  ackpath varchar(15),
			#6  starttime timestamp,
			#7  protocol int unsigned,
			#8  classid int unsigned,	
			#9  netdev varchar(15),
			#10 volume bigint unsigned,
			#11 rate int unsigned,
			#12 dscp int unsigned,
			#13 is_start boolean,
			#14 last_update timestamp
			
			for result in results:
				jobid=result[0]
				sender=result[1]
				receiver=result[2]
				port=result[3]
				datapath=result[4]
				ackpath=result[5]
				protocol=result[7]
				classid=result[8]
				netdev=result[9]
				volume=result[10]
				rate=result[11]
				dscp=result[12]
			
				#Update this job record, mark this job as an executing job and set last_update time as now()
				cur.execute('update job set is_start=1,last_update=now(),starttime=starttime where jobid=%s',(jobid))
			
				#Login receiver to set ACK path ID and start iperf server
				#thread.start_new_thread(start_receiver,(sender,receiver,protocol,port,ackpath,dscp))
				start_receiver(sender,receiver,protocol,port,ackpath,dscp)
			
				#Login sender to set data path ID and start iperf client
				#thread.start_new_thread(start_sender,(sender,receiver,jobid,protocol,port,netdev,classid,datapath,volume,rate,dscp))
				start_sender(sender,receiver,jobid,protocol,port,netdev,classid,datapath,volume,rate,dscp)
			
		#Find a update event need to be executed and its related job
		count=cur.execute('select job.sender,job.receiver,job.protocol,job.port,job.netdev,job.classid,event.eventid,event.jobid,event.datapath,event.ackpath,event.rate,event.dscp \
		                   from job \
						   inner join event \
						   on event.jobid=job.jobid \
						   where event.is_finish=0 and UNIX_TIMESTAMP(now())>UNIX_TIMESTAMP(event.updatetime)')
		if count>0:
			#Get all SQL query results 
			results=cur.fetchall()
			for result in results:
				sender=result[0]
				receiver=result[1]
				protocol=result[2]
				port=result[3]
				netdev=result[4]
				classid=result[5]
				eventid=result[6]
				jobid=result[7]
				datapath=result[8]
				ackpath=result[9]
				rate=result[10]
				dscp=result[11]
			
				#Login in sender to change data path ID, rate and DSCP
				update_sender(sender,receiver,protocol,port,netdev,classid,datapath,rate,dscp)
				#update_sender(sender,receiver,jobid,port,datapath,rate,dscp)
			
				#Login in receiver to change ACK path ID and DSCP
				update_receiver(sender,receiver,protocol,port,ackpath,dscp)
				#update_receiver(sender,receiver,port,ackpath,dscp)
						
				#Update this job record
				cur.execute('update job set datapath=%s, ackpath=%s, rate=%s, dscp=%s, last_update=now(), starttime=starttime where jobid=%s',(datapath,ackpath,rate,dscp,jobid))
			
				#Update this event id
				cur.execute('update event set is_finish=1 where eventid=%s',(eventid))
			
		#Sleep 
		time.sleep(1)
		
	cur.close()
	conn.close()

except MySQLdb.Error,e:
     print "Mysql Error %d: %s" % (e.args[0], e.args[1])