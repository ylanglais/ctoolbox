--
-- Database creation:
-- CREATE DATABASE test;
connect test;
--
-- Users and grants:
-- CREATE USER 'testuser'@'localhost' IDENTIFIED BY 'testuser';
-- GRANT ALL PRIVILEGES ON test.* TO 'testuser'@'localhost' WITH GRANT OPTION;
-- CREATE USER 'testuser'@'%' IDENTIFIED BY 'testuser';
-- GRANT ALL PRIVILEGES ON test.* TO 'testuser'@'%' WITH GRANT OPTION;
--
create table t1 (
	name varchar(10), 
	bval boolean,
	sival smallint,
	ival integer, 
	lval bigint,
	rval real,
	dval double precision
);
insert into t1 values ('zero',  false, 0, 0, 0, 0.0, 0.0);
insert into t1 values ('one',   true,  1, 1, 1, 1.1, 1.1);
insert into t1 values ('two',   true,  2, 2, 2, 2.2, 2.2);
insert into t1 values ('three', true,  3, 3, 3, 3.3, 3.3);
insert into t1 values ('four',  true,  4, 4, 4, 4.4, 4.4);
insert into t1 values ('five',  true,  5, 5, 5, 5.5, 5.5);
insert into t1 values ('six',   true,  6, 6, 6, 6.6, 6.6);


