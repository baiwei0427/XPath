import MySQLdb
import time
import os
import string

try:
	
	#List of end hosts
	hosts=[\
	'192.168.11.1',\
	'192.168.11.2',\
	'192.168.11.3',\
	'192.168.12.1',\
	'192.168.12.2',\
	'192.168.12.3',\
	'192.168.13.1',\
	'192.168.13.2',\
	'192.168.13.3',\
	'192.168.14.1',\
	'192.168.14.2',\
	'192.168.14.3',\
	'192.168.15.1',\
	'192.168.15.2',\
	'192.168.15.3',\
	'192.168.16.1',\
	'192.168.16.2',\
	'192.168.16.3'
	]
	
	#Init results
	results=[]
	
	#Connect database	
	#For RoX 1.2, database is moved from rox to rox3 because table structured has been changed
	conn=MySQLdb.connect(host='localhost',user='root',passwd='root',db='rox3',port=3306)
	cur=conn.cursor()
	
	for host in hosts:
		for i in range(1,51):
			count=cur.execute('select sum(rate) from job where sender=%s and starttime=FROM_UNIXTIME(%s)',(host,210*i))
			if count>0:
				results.append(cur.fetchone()[0])
			count=cur.execute('select sum(rate) from job where receiver=%s and starttime=FROM_UNIXTIME(%s)',(host,210*i))
			if count>0:
				results.append(cur.fetchone()[0])
	
	results.sort(reverse = True)
	print results[0]
	cur.close()
	conn.close()
	
except MySQLdb.Error,e:
     print "Mysql Error %d: %s" % (e.args[0], e.args[1])