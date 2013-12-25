import MySQLdb
import time
import os
import string
import httplib
import urllib
import urllib2

#This function is to start receiver
def start_receiver(sender,receiver,port,ackpath):
	
	#add ACK path ID
	#/home/wei/rox/a.out insert [2] [receiver ip] [sender ip] [6] [port] [0] [ACK path ID] [0] 
	cmd=path+'/a.out insert 2 '+receiver+' '+sender+' '+'6'+' '+str(port)+' '+'0'+' '+ackpath+' '+'0'+':'
	
	#start iperf server
	#python /home/wei/rox/server.py [port]
	cmd=cmd+'python '+path+'/server.py '+str(port)
	
	output=os.popen("python ./controller.py "+receiver+" \'"+cmd+"\'")
	print cmd+"\n"
	print output.read()+"\n"


#This function is to start sender
#Compared with start_receiver it calls for three extra parameters: job id, data volume and sending rate
#tc class of a job=jobid+10
#nfmark for data flow of a job=jobid+10 
#output result file=/home/wei/result/job_[jobid]
def start_sender(sender,receiver,jobid,port,datapath,volume,rate):

	#add data path ID
	#/home/wei/rox/a.out insert [2] [sender ip] [receiver ip] [6] [0] [port] [data path ID] [jobid+10]
	cmd=path+'/a.out insert 2 '+sender+' '+receiver+' '+'6'+' '+'0'+' '+str(port)+' '+datapath+' '+str(jobid+10)+':'

	#add tc class 
	#python /home/wei/rox/tc.py add [interface] [jobid+10] [rate] [jobid+10]
	cmd=cmd+'python '+path+'/tc.py add '+interface+' '+str(jobid+10)+' '+str(rate)+' '+str(jobid+10)+':'
	
	#start iperf client
	#python /home/wei/rox/client.py [receiver ip] [port] [volume] [time interval] [output file]
	cmd=cmd+'python '+path+'/client.py '+receiver+' '+str(port)+' '+str(volume)+' '+str(interval)+' '+output_result+'/job_'+str(jobid)
	
	output=os.popen("python ./controller.py "+sender+" \'"+cmd+"\'")
	print cmd+"\n"
	print output.read()+"\n"

	
#This function is to change configurations at receiver side
#We can only change ACK path ID at receiver side
def update_receiver(sender,receiver,port,ackpath):

	#update ACK path ID
	#/home/wei/rox/a.out update [2] [receiver ip] [sender ip] [6] [port] [0] [ACK path ID] [0] 
	cmd=path+'/a.out update 2 '+receiver+' '+sender+' '+'6'+' '+str(port)+' '+'0'+' '+ackpath+' '+'0';
	
	output=os.popen("python ./controller.py "+receiver+" \'"+cmd+"\'")
	print cmd+"\n"
	print output.read()+"\n"

	
#This function is to change configurations at sender side
#We can change data path ID and rate at sender side
def update_sender(sender,receiver,jobid,port,datapath,rate):

	#update data path ID
	#/home/wei/rox/a.out insert [2] [sender ip] [receiver ip] [6] [0] [port] [data path ID] [jobid+10]
	cmd=path+'/a.out update 2 '+sender+' '+receiver+' '+'6'+' '+'0'+' '+str(port)+' '+datapath+' '+str(jobid+10)+':'
	
	#update tc class 
	#python /home/wei/rox/tc.py change [interface] [jobid+10] [rate]
	cmd=cmd+'python '+path+'/tc.py change '+interface+' '+str(jobid+10)+' '+str(rate)
	
	output=os.popen("python ./controller.py "+sender+" \'"+cmd+"\'")
	print cmd+"\n"
	print output.read()+"\n"

	
try:
	#global variables
	global path,interface,output_result,interval
	#Default directory for rox in each end host
	path='/home/wei/rox'
	#Default network interface rox works on 
	interface='eth1'
	#Default directory for output result in each end host
	output_result='/home/wei/result'
	#Default time interval to record instant throughput
	interval=1

	#Connect database	
	conn=MySQLdb.connect(host='localhost',user='root',passwd='root',db='rox',port=3306)
	cur=conn.cursor()
	
	while(True):
		#Find a new job which is ready to start
		count=cur.execute('select * from job where UNIX_TIMESTAMP(now())>UNIX_TIMESTAMP(starttime) and is_start=0')
		if count>0:
			#Get all SQL query results 
			results=cur.fetchall()
			for result in results:
				jobid=result[0]
				sender=result[1]
				receiver=result[2]
				port=result[3]
				datapath=result[4]
				ackpath=result[5]
				volume=result[7]
				rate=result[8]
			
				#Update this job record, mark this job as an executing job and set last_update time as now()
				cur.execute('update job set is_start=1,last_update=now(),starttime=starttime where jobid=%s',(jobid))
			
				#Insert a data flow record 
				#cur.execute('insert into flow(jobid,type,src_ip,dst_ip,src_port,dst_port,path,rate) \
				#values(%s,%s,%s,%s,%s,%s,%s,%s);',(jobid,'1',sender,receiver,'0',port,datapath,rate))
			
				#Insert a ACK flow record
				#cur.execute('insert into flow(jobid,type,src_ip,dst_ip,src_port,dst_port,path,rate) \
				#values(%s,%s,%s,%s,%s,%s,%s,%s);',(jobid,'2',receiver,sender,port,'0',ackpath,'0'))
			
				#Login receiver to set ACK path ID and start iperf server
				start_receiver(sender,receiver,port,ackpath)
			
				#Login sender to set data path ID and start iperf client
				start_sender(sender,receiver,jobid,port,datapath,volume,rate)
			
		#Find a update event need to be executed and its related job
		count=cur.execute('select job.sender,job.receiver,job.port,event.eventid,event.jobid,event.datapath,event.ackpath,event.rate \
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
				port=result[2]
				eventid=result[3]
				jobid=result[4]
				datapath=result[5]
				ackpath=result[6]
				rate=result[7]
			
				#Login in sender to change data path ID and rate
				update_sender(sender,receiver,jobid,port,datapath,rate)
			
				#Login in receiver to change ACK path ID 
				update_receiver(sender,receiver,port,ackpath)
			
				#Update data flow record
				#cur.execute('update flow set path=%s, rate=%s where jobid=%s and type=1',(datapath,rate,jobid))

				#Update ACK flow record
				#cur.execute('update flow set path=%s where jobid=%s and type=2',(ackpath,jobid))
						
				#Update this job record
				cur.execute('update job set datapath=%s, ackpath=%s, rate=%s, last_update=now(), starttime=starttime where jobid=%s',(datapath,ackpath,rate,jobid))
			
				#Update this event id
				cur.execute('update event set is_finish=1 where eventid=%s',(eventid))
			
		#Sleep 
		time.sleep(1)
		
	cur.close()
	conn.close()

except MySQLdb.Error,e:
     print "Mysql Error %d: %s" % (e.args[0], e.args[1])