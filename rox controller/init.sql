create database if not exists rox;

use rox;

create table job
(
	jobid int unsigned auto_increment primary key,
	sender varchar(15),
	receiver varchar(15),
	port int unsigned,
	datapath varchar(15),
	ackpath varchar(15),
	starttime timestamp,
	volume int unsigned,
	rate int unsigned,
	is_start boolean,
	last_update timestamp
);

create table flow
(
	flowid int unsigned auto_increment primary key,
	jobid int unsigned,
	type int unsigned,
	src_ip varchar(15),
	dst_ip varchar(15),
	src_port int unsigned,
	dst_port int unsigned,
	path varchar(15),
	rate int unsigned
);

create table event
(
	eventid int unsigned auto_increment primary key,
	jobid int unsigned,
	updatetime timestamp,
	datapath varchar(15),
	ackpath varchar(15),
	rate int unsigned,
	is_finish boolean
);

#create table mapping
#(
#	mappingid int unsigned auto_increment primary key,
#	host int unsigned,
#	vm int unsigned,
#	path int unsigned
#);
