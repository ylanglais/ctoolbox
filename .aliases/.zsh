#
#       Define aliases and functions:
#

#alias | grep rm 1>/dev/null 2>&1 && unalias rm
alias | grep "^mv=" 1>/dev/null 2>&1 && unalias mv
alias | grep "^cp=" 1>/dev/null 2>&1 && unalias cp
#
alias j=jobs
alias grep="grep --color"
alias egrep="egrep --color"
alias fgrep="fgrep --color"
alias ls="ls -CF --color=auto"
alias l=" ls -CFA --color=auto"
alias la="ls -CFAl --color=auto"
alias ll="ls -CFl --color=auto"
alias lt="ls -CFlrt --color=auto"
alias lS='ls -lS --color'
alias lrs='ls -lrS  --color'
alias las='ls -alS  --color'
alias lar='ls -alrS --color'

alias diff='diff --color=auto'

alias h="history -1000"
alias hg="history -1000 | grep"
alias hh="cat ~/.histfiles/* | grep "
alias pss='ps auxw | egrep $(whoami)"|USER" | egrep -v "grep|ps auxw" | sort -n +1'

alias hd='hexdump -Cv'
alias make='make --no-print-directory'
alias wg="wget -l2 -rF"
alias g='wget -l2 -rF -A mpg'
alias v='wget -l2 -rF -A wmv'
#
# Git aliases:
alias gsta='git status'
alias gdif='git diff'
alias glog='git log'
alias gco='git checkout'
alias gci='git commit'
#
rot13() {    
    if [ $# = 0 ]
	then
        tr "[a-m][n-z][A-M][N-Z]" "[n-z][a-m][N-Z][A-M]"  
    else
        tr "[a-m][n-z][A-M][N-Z]" "[n-z][a-m][N-Z][A-M]" < $1
    fi
}  
flds() {
	head -1 $1 | sed -e "s/;/\n/g" | nl
}
fldnums() {
	head -1 $1 | sed -e "s/;/\n/g" | nl | egrep $2 | tr -s " 	" " " | cut -d" " -f 2 | sed -e "s/\n/,/" | tr '\n' ',' | sed -e 's/,$//'
}
hosts() {
	#
	# Special own aliases:
	# Translate host db into aliases:
	for i in $(cat  ~/.hlist) 
	do
		port=""
		echo $i | egrep "^([^     ]){1,}:([^      ]*){1,}(:([0-9]{1-5})){0,1}" 1>/dev/null 2>&1 && {
			k=$(echo $i|cut -d":" -f 1)
			v=$(echo $i|cut -d":" -f 2)
			h=$(echo $v|cut -d"@" -f 2)
			p=$(echo $i|cut -d:   -f 3)
			[ -z $p ] || port="-p $p"
			eval "alias $k='xnames ssh $v; ssh $port $v;xnames \"$(hostname)-${Shell}-${SessionID} $(basename ${PWD})\"'"
			eval "alias _$k='echo $v'"
		}
	done
}
#
psg() {
    ps -auxx | grep `whoami` | grep $1 | grep -v grep
}   

chpwd () {
	Pwd=`pwd`
	xtitle "`hostname`-${Shell}-${SessionID} ${Pwd}"
	xicon "`hostname`-${Shell}-${SessionID} `basename ${Pwd}`"
	\ls -CF --color=auto
}
function ff {
    find . -name $* -print
}
ssg () {
	export xns=$_xnames; 
	(xtitle "$*i"; xicon "$*i") 1>/dev/null 2>&1 
	ssh $*
	xnames $xns
}
ssi() {
	ssh -p 55555 ilay.org 
}
tfe() {
	[ -d ./logs -a -f logs/error.log ] && {
		(
			export xns=$_xnames; 
			ctrlc() { xnames $xns }
			trap ctrlc INT
			xnames "$PRJ: >>>>  logs/error.log" ; 
			tail -f logs/error.log
		)
	} || echo "No logs/error.log file to tail -f"
}
tfa() {
	[ -d ./logs -a -f logs/access.log ] && {
		(
			export xns=$_xnames; 
			ctrlc() { xnames $xns }
			trap ctrlc INT
			xnames "$PRJ: >>>>  logs/access.log" ; 
			tail -f logs/access.log
		)
	} || echo "No logs/error.log file to tail -f"
}
tfl() {
	[ -d ./logs  ] && {
		(
			export xns=$_xnames; 
			ctrlc() { xnames $xns }
			trap ctrlc INT
			xnames "$PRJ: >>>>  logs/*.log" ; 
			tail -f logs/*.log
		)
	} || echo "No logs/ directory -f"
}


sete() {
	e=$1
	[ -f ~/.prjs ] || {
		echo $bold${red}No project definiton${norm}
	} 
	export PRJ=$e
	export DBNAME=$(grep "^$e:" ~/.prjs|cut -d: -f2)
	export PRJDIR=$(eval echo $(grep "^$e:" ~/.prjs|cut -d: -f3))
	export PRJENV=$(grep "^$e:" ~/.prjs|cut -d: -f4)
	cd $PRJDIR
}
_sete() {
	reply=( `grep -v "^#" ~/.prjs |cut -d: -f 1 `)
}
dbname() {
	[ -z $DBNAME ] || { 
		echo "$white$bold$DBNAME$norm " 
	}
}
prj() {
	[ x"$PRJ" != x""  ] && { 
		c_="";
		p_=""
		[ -z "$PRJENV" ] || {
			c_="$bold"
			[ "$PRJENV" == "dev" ] && {
				c_="$bold$green"
			} || {
				[ "$PRJENV" == "rec" ] && c_="$bold$yellow" || c_="$bold$red"
			} 
			p_=":$c_$PRJENV$norm"
		}	
		echo "$white$bold$PRJ$norm$p_ " 
	} || {
		dbname
	}	
}
#
#
dsvg() {
	[ $# -lt 1 ] && {
		echo "specify directory to save"
		return
	} 
	rep=$1
	suf=""
	[ $# -gt 1 ] && suf=".$2"
	d=`date +"%Y%m%d"`
	name=$rep.$d$suf.tgz
	[ -e $name ] && {
		val=$(ls -C1 $rep.$d.[0-9]{1,}$suf.tgz 2>/dev/null | tail -1 | sed -e "s/^$rep.$d.\([0-9]\{1,\}\)$suf.tgz$/\1/g")
		[ -z $val ] && val=1 || let val=val+1
		name=$rep.$d.$val$suf.tgz 	
	}
	echo "Saving $bold$green$1$norm directory to "$bold$yellow$name$norm
	tar chzf $name $rep
}
_dsvg() { reply=(`ls`); }
lsdb() {
	cut -d":" -f1 $HOME/.dbs
}
_lsdb() {
	reply=(`lsdb`)
}
setdb() {
	sete
	export DBNAME=$1
}
__db_check() {
	[ -z $DBNAME ] && {
		echo "${bold}${red}No database$norm"
		return 1	
	}	
	a=$(grep "^$DBNAME:" $HOME/.dbs)
	[ $? -ne 0 ] && {
		echo "${bold}${red}Invalid databse name, unsetting$norm"
		unset $DBNAME
		return 2
	}
	ds=$(__get_ds)
	a=$(grep "^$ds:" $HOME/.ds)
	[ $? -ne 0 ] && {
		echo "${bold}${red}Invalid dataserver name$norm"
		return 3
	}
	export ds=$(grep "^$DBNAME:" $HOME/.dbs | cut -d":" -f3)

}
__get_ds() {
	grep "^$DBNAME:" $HOME/.dbs | cut -d":" -f3
}
__get_dn() {
	grep "^$DBNAME:" $HOME/.dbs | cut -d":" -f2
}
__get_dt() {
	grep "^$DBNAME:" $HOME/.dbs | cut -d":" -f4
}
__get_dh() {
	[ -z $ds ] && return 4 
	grep "^$ds:" $HOME/.ds | cut -d":" -f2
}
__get_dp() {
	[ -z $ds ] && return 5 
	grep "^$ds:" $HOME/.ds | cut -d":" -f3
}
__get_dl() {
	[ -z $ds ] && return 6 
	grep "^$ds:" $HOME/.ds | cut -d":" -f4
}
__get_dw() {
	[ -z $ds ] && return 7 
	grep "^$ds:" $HOME/.ds | cut -d":" -f5
}
dbdmp() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	d=`date +"%Y%m%d"`
	name=$dn.$d.sql 	
	[ -e $name ] && {
		val=$(ls -C1 $dn.$d.[0-9]{1,}.sql 2>/dev/null | tail -1 | sed -e "s/^$dn.$d.\([0-9]\{1,\}\).sql$/\1/g")
		[ -z $val ] && val=1 || let val=val+1
		name=$dn.$d.$val.sql 	
	}
	echo "Dump database $bold$green$dn$norm to $bold$yellow$name$norm"

	[ $dt = "mysql" ] && {
		mysqldump -h $dh -u $dl -p$dw -P $dp --lock-tables=false $dn $* > $name
		true
	} || {
		PGPASSWORD=$(__get_dw) pg_dump -h$dh $dn -U $dl $* > $name
	}
}
dbdmpcmd() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	d=`date +"%Y%m%d"`
	name=$dn.$d.sql 	
	[ -e $name ] && {
		val=$(ls -C1 $dn.$d.[0-9]{1,}.sql 2>/dev/null | tail -1 | sed -e "s/^$dn.$d.\([0-9]\{1,\}\).sql$/\1/g")
		[ -z $val ] && val=1 || let val=val+1
		name=$dn.$d.$val.sql 	
	}
	echo "Dump database $bold$green$dn$norm to $bold$yellow$name$norm"

	[ $dt = "mysql" ] && {
		echo mysqldump -h $dh -u $dl -p$dw -P $dp --lock-tables=false $dn $* > $name
	} || {
		echo PGPASSWORD=$(__get_dw) pgdump -h$dh $dn -U $dl $* > $name
	}
}
dbschema() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	d=`date +"%Y%m%d"`
	name=$dn.schema.$d.sql 	
	[ -e $name ] && {
		val=$(ls -C1 $dn.$d.[0-9]{1,}.sql 2>/dev/null | tail -1 | sed -e "s/^$dn.$d.\([0-9]\{1,\}\).sql$/\1/g")
		[ -z $val ] && val=1 || let val=val+1
		name=$dn.$d.$val.sql 	
	}
	echo "Dump database $bold$green$dn$norm to $bold$yellow$name$norm"

	[ $dt = "mysql" ] && {
		mysqldump -h $dh -u $dl -p$dw -P $dp -d --lock-tables=false $dn $* > $name
		true
	} || {
		PGPASSWORD=$(__get_dw) pg_dump -s -h$dh $dn -U $dl $* > $name
	}
}
pg() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	[ -z "$DBNOPAD" ] && { pad="" } || { pad="-A"        }
	[ -z "$DBSEP"   ] && { sep="" } || { sep="-R $DBSEP" }
	[ -z "$DBNOTI" ]  && { ti=""  } || { ti="-t"         }
	[ $# -ge 1 ] && {
		PGPASSWORD=$(__get_dw) psql -h$dh $dn -U $dl -P pager=off -A $sep $pad $ti -c "$*" 
		true
	} || {
		PGPASSWORD=$(__get_dw) psql -h$dh $dn -U $dl -P pager=off $sep $pad $ti
	}
}
msql() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	[ -z "$DBNOPAD" ] && { pad="" } || { pad="-B"      }
	[ -z "$DBNOTI"  ] && { ti=""  } || { ti="--skip-column-names" }
	[ $# -ge 1 ] && {
		mysql  $pad $ti -h $dh -u $dl -p$dw -P $dp $dn -e "$*" $sep
		true
	} || {
		mysql  $pad $ti -h $dh -u $dl -p$dw -P $dp $dn 
		true
	}
}
xnrst() {
	[[ -v PRJ ]] && sete $PRJ || {
		[[ -v DBNAME ]] && setdb $DBNAME || {
			xnames "$(hostname)-${Shell}-${SessionID} $(basename ${PWD})"
		}
	}
}
sql() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	export xns=$_xnames; 
	ctrlc() { xnames $xns }
	trap ctrlc INT
	xnames "$(hostname) ${Shell}-${SessionID} sql $dn"
	[ $dt = "mysql" ] && {
		[ -z "$DBNOPAD" ] && { pad="" } || { pad="-B"      }
		[ -z "$DBNOTI"  ] && { ti=""  } || { ti="--skip-column-names" }
		[ $# -ge 1 ] && {
			mysql  $pad $ti -h $dh -u $dl -p$dw -P $dp $dn -e "$*" $sep
			true
		} || {
			mysql  $pad $ti -h $dh -u $dl -p$dw -P $dp $dn 
			true
		}
	} || {
		[ -z "$DBNOPAD" ] && { pad="" } || { pad="-A"        }
		[ -z "$DBSEP"   ] && { sep="" } || { sep="-R $DBSEP" }
		[ -z "$DBNOTI" ]  && { ti=""  } || { ti="-t"         }
		[ $# -ge 1 ] && {
			PGPASSWORD=$(__get_dw) psql -h$dh -p $dp $dn -U $dl -P pager=off -A $sep $pad $ti -c "$*" 
			true
		} || {
			PGPASSWORD=$(__get_dw) psql -h$dh -p $dp $dn -U $dl -P pager=off $sep $pad $ti
		}
	}
	xnames $xns
}
sqlcmd() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	[ $dt = "mysql" ] && {
		[ -z "$DBNOPAD" ] && { pad="" } || { pad="-B"      }
		[ -z "$DBNOTI"  ] && { ti=""  } || { ti="--skip-column-names" }
		[ $# -ge 1 ] && {
			echo mysql  $pad $ti -h $dh -u $dl -p$dw -P $dp $dn -e "$*" $sep
			true
		} || {
			echo mysql  $pad $ti -h $dh -u $dl -p$dw -P $dp $dn 
			true
		}
	} || {
		[ -z "$DBNOPAD" ] && { pad="" } || { pad="-A"        }
		[ -z "$DBSEP"   ] && { sep="" } || { sep="-R $DBSEP" }
		[ -z "$DBNOTI" ]  && { ti=""  } || { ti="-t"         }
		[ $# -ge 1 ] && {
			echo PGPASSWORD=$(__get_dw) psql -h$dh $dn -U $dl -P pager=off -A $sep $pad $ti -c "$*" 
			true
		} || {
			echo PGPASSWORD=$(__get_dw) psql -h$dh $dn -U $dl -P pager=off $sep $pad $ti
		}
	}
}

#
#	db stuff
#
qw() {
	__db_check || return $?
	ds=$(__get_ds)
	dn=$(__get_dn)
	dt=$(__get_dt)
	dh=$(__get_dh)
	dp=$(__get_dp)
	dl=$(__get_dl)
	dw=$(__get_dw)

	[ $dt = "mysql" ] && {
		mysql  -h $dh -u $dl -p$dw -P $dp $dn -rs --disable-pager -e "$*"
		true
	} || {
		PGPASSWORD=$(__get_dw) psql -h$dh $dn -U $dl -P pager=off $sep -t -c "$*" | grep -v "utilisation du paginateur" | sed -e 's/ | /\t/g' 
	}
}
tables() {
	tab=""
	dn=$(__get_dn)
	[ $# -eq 0 ] && {
		echo "$yellow$bold"Tables in $dn $norm  
	} || {
		echo "$yellow$bold"Tables in $dn like '$1'$norm 
		tab=" and table_name like '$1'";
	}
	qw "select distinct table_name from information_schema.columns where table_schema = (select database())$tab" | grep -v "^table_name$"
}
cols() {
	case $# in
	0) 
		echo "no table given"
		return 1
		;;
	1) 
		tab="table_name like '$1'"
		;; 
	*)
		tab=""
		for i in $* 
		do
			[ -z $tab ] || tab="$tab,"
			tab="$tab'$i'" 
		done
		tab="table_name in ($tab)"
	esac
	echo "$yellow$bold"Table	Column$tab$norm
	qw "select table_name, column_name from information_schema.columns where table_schema = (select database()) and $tab"  | grep -v "^column_name$"
}
wtab() {
	echo "$yellow$bold"Table	Column$norm
	[ $# -eq 1 ] && {
		qw "select table_name, column_name from information_schema.columns where table_schema = (select database()) and column_name like '%$1%'" | grep -v "^column_name$"
		return 0
	}
	for col in $*
	do 
		echo "$bold$col:$norm"
		qw "select table_name, column_name from information_schema.columns where table_schema = (select database()) and column_name like '%$col%'" | grep -v "^column_name$"
	done
}
lldb() {
	__db_check || return $?
	echo "$yellow$bold"Databases$norm
	[ $(__get_dt) = "mysql" && { 	
		qw show databases | head -n -1 | sort
		true
	} || {
		qw "\dt"
	}
}
#compdef setdb
_lldb() {
	reply=(`lldb`)
}
#
#
setldb() {
	sete
	export DBNAME=$1
}
#compdef setdb
_lldb() {
	reply=(`lldb`)
}
#
#
lldb() {
	qw show databases | head -n -1 | sort
}
_cols() {
	reply=(`tables`)
}
#
#	Dump a database:
#
dumpdb() {
	[ $# -lt 1 ] && {
		echo "${yellow}dumpdb usage:$norm"
		echo "dumpdb ${yellow}dbname $blue[suffix]$norm"
		return 1
	}	 
	dbname=$1
	[ $# -gt 1 ] && suffix=".$2" || suffix=""
	
	d=`date +"%Y%m%d"`
	name=$dbname.$d$suffix.sql 	
	[ -e $name ] && {
		val=$(ls -C1 $dbname.$d.[0-9]{1,}$suffix.sql 2>/dev/null | tail -1 | sed -e "s/^$dbname.$d.\([0-9]\{1,\}\)$suffix.sql$/\1/g")
		[ -z $val ] && val=1 || let val=val+1
		name=$dbname.$d.$val$suffix.sql 	
	}
	echo "Dump database $bold$green$dbname$norm to $bold$yellow$name$norm"
	mysqldump $dbname > $name
}

#
#	Dump a database:
#
dumpmodel() {
	[ $# -lt 1 ] && {
		echo "${yellow}dumpmodel usage:$norm"
		echo "dumpmodel ${yellow}dbname $blue[suffix]$norm"
		return 1
	}	 
	dbname=$1
	[ $# -gt 1 ] && suffix=".$2" || suffix=""
	
	d=`date +"%Y%m%d"`
	name=$dbname.model.$d$suffix.sql 	
	[ -e $name ] && {
		val=$(ls -C1 $dbname.model.$d.[0-9]{1,}$suffix.sql 2>/dev/null | tail -1 | sed -e "s/^$dbname.model.$d.\([0-9]\{1,\}\)$suffix.sql$/\1/g")
		[ -z $val ] && val=1 || let val=val+1
		name=$dbname.model.$d.$val$suffix.sql 	
	}
	echo "Dump database model $bold$green$dbname$norm to $bold$yellow$name$norm"
	mysqldump -d $dbname > $name
}

#
# truncate database after saving it;
#
truncatedb() {
	[ $# -lt 1 ] && {
		echo "${yellow}truncatedb usage:$norm"
		echo "truncatedb ${yellow}dbname$norm"
		return 1
	}	 
	dbname=$1
	dumpdb $dbname before_truncation
	echo "Truncate database $bold$green$dbname$norm"
	mysql -e "drop database $dbname; create database $dbname"
}

[ -f ~/.hosts ] && . ~/.hosts

#
#
br() {
	echo ""
}
rcmd() {
	local host=$1; shift
	( ssh $host $* ) 2>&1 | tr -d "\r" 
}
function dirdiff() {
	local hd1=$(echo $1)
	local hd2=$(echo $2)
	local d1 h1 cmd1 l1
	local d2 h2 cmd2 l2
	echo $hd1 | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		d1=$(echo $hd1| cut -d":" -f2-)
		h1=$(echo $hd1| cut -d":" -f1)
		cmd1="ssh $h1 \ls -C1F $d1"
		true
	} || {
		d1=$hd1
		cmd1="\ls -C1F $d1"
	}
	echo $hd2 | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		d2=$(echo $hd2| cut -d":" -f2-)
		h2=$(echo $hd2| cut -d":" -f1)
		cmd2="ssh $h2 \ls -C1F $d2"
		true
	} || {
		d2=$hd2
		cmd2="\ls -C1F $d2"
	}
	echo "$bold$yellow""Files in $hd1, but not un $hd2:""$norm"
	l2=$(eval $cmd2);
	for i in $(eval $cmd1)
	do
		echo $l2 | grep $i >/dev/null 2>&1 || {
			echo $i
		}
	done  
	echo "$bold$yellow""Files in $hd2, but not un $hd1:""$norm"
	l1=$(eval $cmd1);
	for i in $(eval $cmd2)
	do
		echo $l1 | grep $i >/dev/null 2>&1 || {
			echo $i
		}
	done  
}
date_size() {
	local ff=$(echo $1)
	local f1 h1
	echo $ff | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		f1=$(echo $ff| cut -d":" -f2-)
		h1=$(echo $ff| cut -d":" -f1)
		echo -n $(ssh $h1 "ls -l --time-style=+\"%Y/%m/%d %H:%M:%S\" $f1 | cut -c 34-")
	} || echo -n $(ls -l --time-style=+"%Y/%m/%d %H:%M:%S" $ff | cut -c 34-)
}
fexist() {
	local ff=$(echo $1)
	local f1 h1
	echo $ff | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		f1=$(echo $ff| cut -d":" -f2-)
		h1=$(echo $ff| cut -d":" -f1)
		ssh $h1 "test -f $f1"
		return $?
	} 
	
	test -f $ff 
	return $?
}
dexist() {
	local ff=$(echo $1) 
	local f1 h1
	echo $ff | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		f1=$(echo $ff| cut -d":" -f2-)
		h1=$(echo $ff| cut -d":" -f1)
		ssh $h1 "test -d $f1 -a \! -h $f1" 
		return $?
	} 
	test -d $ff -a \! -h $ff 
	return $?
}
differ() {
	local rep1=$(echo $1)
	local rep2=$(echo $2)
	local r1 h1 hh1 l1
	local r2 h2 hh2 l2
	local i f
	local cmd1 cmd2
	local dif dlc

	echo $rep1 | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		r1=$(echo $rep1| cut -d":" -f2-)
		h1=$(echo $rep1| cut -d":" -f1)
		hh1="$h1:"
		r1=$(ssh $h1 "echo $r1")
		rep1=$hh1$r1
		l1="ssh $h1 \ls -a $r1"
	} || {
		r1=$rep1
		h1=""
		l1="\ls -aC1 $r1 | grep -vE '^\.$|^\.\.$'"
	}
	
	echo $rep2 | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		r2=$(echo $rep2| cut -d":" -f2-)
		h2=$(echo $rep2| cut -d":" -f1)
		r2=$(ssh $h2 "echo $r2")
		hh2="$h2:"
		rep2=$hh2$r1
		l2="ssh $h2 \ls -Qb $r2"
	} || {
		r2=$rep2
		h2=""
		hh2=""
		l2="\ls -Qb $r2"
	}

	#echo differ $hh1$r1 $hh2$r2

	#dirdiff $rep1 $rep2
	for i in $(eval $l1)
	do
		f=$(echo $i | sed -e "s|$r1/||")
		fexist $rep2/$f && {
			#echo ">>>> $rep2/$f exists"
			[ "$h1" != "" ] && cmd1="ssh $h1 cat $r1/$i" || cmd1="cat $r1/$i"
			[ "$h2" != "" ] && cmd2="ssh $h2 cat $r2/$f" || cmd2="cat $r2/$f"

			dif="diff -w <(eval $cmd1) <(eval $cmd2)" 

			echo -n "$yellow"diff $rep1/$i $rep2/$f ...$norm
			eval $dif 1>/dev/null 2>&1
			[ $? -ne 0 ] && {
				echo "$red$bold"KO"$norm";
				echo "< "$cyan$(date_size $rep1/$i)$norm
				echo "> "$cyan$(date_size $rep2/$f)$norm
				diffstr=$(eval $dif)
				dnl=$(echo $diffstr | wc -l)
				[ $dnl -gt 30 ] && {
					echo $diffstr | head -n 10
					echo "$blue (...output trucated after first 10 lines...) $norm"
				} || echo $diffstr
				true
			} || echo "$green$bold"OK"$norm"
		} || {
			dexist $rep1/$i && { 
				dexist $rep2/$f && { 
					echo "$yellow$bold"Entering $rep1/$i$norm
					differ  $rep1/$i $rep2/$f 
					#echo "$yellow$bold"Entering $rep1/$(dirname $i)$norm
					true
				} || {
					echo "$bold$red}Directory $rep2/$f doesn't exist$norm"  
				}
				true
			} || echo "$bold$red$rep1/$i has no counterpart in $(dirname $rep2/$i) $norm"
		}
	done
}

tdiffer() {
	rep1=$(echo $1)
	rep2=$(echo $2)
	echo $rep1 | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		r1=$(echo $rep1| cut -d":" -f2-)
		h1=$(echo $rep1| cut -d":" -f1)
		r1=$(ssh $h1 "echo $r1")
		l1="ssh $h1 find $r1 -type f"
		d1="ssh $h1 find $r1 -type d"
	} || {
		r1=$rep1
		h1=""
		l1="find $r1 -type f"
		d1="find $r1 -type d"
	}
	
	echo $rep2 | egrep "^[a-zA-Z0-9@._-]*:.*$" >/dev/null 2>&1 && {
		r2=$(echo $rep2| cut -d":" -f2-)
		h2=$(echo $rep2| cut -d":" -f1)
		r2=$(ssh $h2 "echo $r2")
		hh2="$h2:"
		l2="ssh $h2 find $r2 -type f"
	} || {
		r2=$rep2
		h2=""
		hh2=""
		l2="find $r2 -type f"
	}

	#dirdiff $rep1 $rep2

	for i in $(eval $l1)
	do
		f=$(echo $i | sed -e "s|$r1/||")
		fexist $rep2/$f && {
			[ "$h1" != "" ] && cmd1="ssh $h1 cat $i"     || cmd1="cat $i"
			[ "$h2" != "" ] && cmd2="ssh $h2 cat $r2/$f" || cmd2="cat $r2/$f"

			dif="diff -w <(eval $cmd1) <(eval $cmd2)" 

			echo -n "$bold$yellow"diff $hh1$i $rep2/$f ...$norm
			eval $dif 1>/dev/null 2>&1
			[ $? -ne 0 ] && {
				echo "$red$bold"KO"$norm";
				echo "< "$cyan$(date_size $hh1$i)$norm
				echo "> "$cyan$(date_size $rep2/$f)$norm
				eval $dif
				true
			} || {
				echo "$green$bold"OK"$norm"
			} 
		} || {
			#dexist $rep2/$i || 
			echo "$bold$red$hh1$i has no counterpart in $rep2/$i $norm"
		}
	done
}
rdiffer() {
	host1=$1
	host2=$2
	file=$3;
	chk=$(rcmd $host1 ls $file | tr -d '\r') 
	if [ "$chk" == "$file" ] 
	then 
		echo "$cyan$file diff between $host1 and $host2:$norm"
		(rcmd $host1 cat $file) > $host1.tmp
		(rcmd $host2 cat $file) > $host2.tmp
		diff -w $host1.tmp $host2.tmp && echo "$green"files are identical$norm
		return $?
	else 
		echo "${red}No $file on $host1$norm"
	fi
}
rdiffdir() {
	host1=$1
	host2=$2
	dir=$3;
	echo "$cyan$dir directory diff between $host1 and $host2:$norm"
	(rcmd $host1 "cksum $dir/* 2>/dev/null")|sort -k 3 > $host1.tmp
	(rcmd $host2 "cksum $dir/* 2>/dev/null")|sort -k 3 > $host2.tmp
	diff -w $host1.tmp $host2.tmp && echo "$green"directories are identical 
	return $?
}
rdiffcmd() {
	host1=$1; shift
	host2=$1; shift
	cmd=$*;
	echo "$cyan$cmd diff between $host1 and $host2:$norm"
	(rcmd $host1 $cmd) > $host1.tmp
	(rcmd $host2 $cmd) > $host2.tmp
	diff -w $host1.tmp $host2.tmp && echo "$green"cmd results are identical$norm
}

mdays() {
	while [ -n "$1" ] 
	do  
		echo "$1": $(( ( $(date +%s) - $(stat -c %Y "$1") ) / 3600 / 24 ))
		shift
	done
}
#
compctl -K _sete sete
compctl -K _lldb lldb
compctl -K _lsdb lsdb
compctl -K _lsdb setdb
compctl -K _lsdb dbdmp
compctl -K _lsdb dbschema
compctl -K _lldb setldb
compctl -K _lldb dumpdb
compctl -K _lldb mysql
compctl -K _lldb mysqldump
compctl -K _lldb dumpmodel
compctl -K _lldb truncatedb
compctl -K _cols cols

[ -f ~/.aliases/.docker ] && . ~/.aliases/.docker
[ -f ~/.aliases/.work   ] && . ~/.aliases/.work
