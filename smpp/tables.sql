create table sms_sendqueue (
	id	  	varchar(20)	unique not null,
	mid	  	varchar(20),
	fromnum varchar(20) not null,
	tonum 	varchar(20)	not null,
	status	char(20) not null,
	sent	char(20),
	acked   char(20),
	final   char(20)
);


