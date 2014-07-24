create database if not exists rox3;

use rox3;

create table job
(
	jobid int unsigned auto_increment primary key,
	sender varchar(15),
	receiver varchar(15),
	port int unsigned,
	datapath varchar(15),
	ackpath varchar(15),
	starttime timestamp,
	protocol int unsigned,
	classid int unsigned,	
	netdev varchar(15),
	volume bigint unsigned,
	rate double,
	dscp int unsigned,
	is_start boolean,
	last_update timestamp
);

create table event
(
	eventid int unsigned auto_increment primary key,
	jobid int unsigned,
	updatetime timestamp,
	datapath varchar(15),
	ackpath varchar(15),
	rate double,
	dscp int unsigned,
	is_finish boolean
);

jobid: id of job, a global variable
sender: IP address of sender (iperf client) 
receiver: IP address of receiver (iperf server)
port: TCP port that iperf servers listen on (e.g. 5001)
datapath: path ID for data traffic (from sender to receiver)
ackpath: path iD for ACK traffic (from receiver to sender)
starttime: when to start this job
protocol: TCP(6) or UDP (17)
classid: tc class id for traffic shaping (rate limiting) in sender side (used by tc htb class). Two jobs cannot share the equal classid in a NIC of a server.
netdev: network device name of sender to send this job (e.g. eth0, eth1...)
volume: Size of traffic to send
rate: sending rate
dscp: value of Differentiated Services Code Point
is_start: whether this job has beem started
last_update: last update time (e.g. start job, update job setting)

You can only change datapath, ackpath, rate ,dscp of a job via event.




