#!/bin/zsh 
##########################################################################################
# 
#
# On interactive Shells Only!!!!: (file ~/.zshrc)
#
#
##########################################################################################
hasxset=$( test -x /usr/bin/xset && echo 1 || echo 0 )
[ $hasxset -eq 1 ] && xset -b
bindkey -v
#
# Get a new session number:
#
#export DISPLAY=:0.0
export Shell=zsh
PROCESSID=$$; export PROCESSID
. $HOME/.aliases/.$Shell
[ x"$REQSSID" != x ] && {
	SessionID=$REQSSID
	unset REQSSID
} || { 
	exec 3>&1
	pid=$$
	SessionID=$(~/scripts/session_new $pid)
	#SessionID=`get_session`
}
export SessionID
#echo "$SessionID -> $HOME/.histfiles/.$Shell.$HOST.$SessionID"
#
#
#
setopt INC_APPEND_HISTORY
setopt APPEND_HISTORY
HISTFILE=$HOME/.histfiles/.$Shell.$HOST.$SessionID; export HISTFILE
#
#
# Prevents waiting for display of C-Shell messages.
#
set notify
#
#
# Text options:
export over=$(tput smso)
export noover=$(tput rmso)
export bold=$(tput bold)
export nobold=$(tput sgr0)
export blink=$(tput blink)
export noblink=$(tput sgr0)
export italic=$(tput sitm)
export noitalic=$(tput ritm)
export under=$(tput smul)
export nounder=$(tput rmul)
export norm=$(tput sgr0)
#
# Foreground colors:
export black=$(tput setaf 0)
export red=$(tput setaf 1)
export green=$(tput setaf 2)
export yellow=$(tput setaf 3)
export blue=$(tput setaf 4)
export magenta=$(tput setaf 5)
export cyan=$(tput setaf 6)
export white=$(tput setaf 7)
#
# Background colors:
export bgblack=$(tput setab 0)
export bgred=$(tput setab 1)
export bggreen=$(tput setab 2)
export bgyellow=$(tput setab 3)
export bgblue=$(tput setab 4)
export bgmagenta=$(tput setab 5)
export bgcyan=$(tput setab 6)
export bgwhite=$(tput setab 7)
#
# Parameters:
cdpath=(.)  
fignore=(.o \~ .BAK .bak OLD old sav)
setopt prompt_subst
#PROMPT="%(?..%{[1;31m%}Error: %?
#)
#%{[1;35m%}%h%{[1;32m%} $Shell-$SessionID \$(prj)%{[1;34m%}%n@%M:%{[1;33m%}%/%{[1;30m%}%b         
#"
PROMPT="%(?..$bold${red}Error: %?
)
$bold${magenta}%h$green $Shell-$SessionID \$(prj)${blue}%n@%M:${yellow}%/$norm%b
"

#RPROMPT="%B%{[1;35m%}%D{%H:%M:%S %d/%m/%Y}%{[1;30m%}%b"
PROMPT2="$bold$blue"
PROMPT3="$bold$magenta"
PROMPT4="+ "
SPROMPT=" ${bold}${green}CORRECT %R to %r (y|[n]|e|a):$norm%b "
SAVEHIST=500
#HISTFILE=$HOME/.histfiles/.h_zsh_$HOST"_"$SESSIO$normN
HISTSIZE=500
LISTMAX=10000
export HELPDIR=$HOME/lib/zsh/help
autoload run-help
autoload zed
hosts=(`hostname` prep.ai.mit.edu wuarchive.wustl.edu gatekeeper.dec.com)
WORDSHAR="_-.#~(){}[]"
#
# Options:
#
setopt  ALWAYS_TO_END AUTO_CD AUTO_LIST AUTO_PARAM_KEYS AUTO_PUSHD AUTO_REMOVE_SLASH 
setopt  BRACE_CCL CDABLE_VARS COMPLETE_ALIASES COMPLETE_IN_WORD CORRECT 
setopt  CSH_NULL_GLOB GLOB_COMPLETE GLOB_DOTS HASH_CMDS HASH_DIRS 
setopt  HIST_IGNORE_DUPS HIST_NO_STORE IGNORE_EOF INTERACTIVE_COMMENTS LIST_AMBIGUOUS 
setopt  LIST_TYPES LONG_LIST_JOBS NO_BAD_PATTERN NO_LIST_BEEP NO_NOMATCH NOTIFY 
setopt  NUMERIC_GLOB_SORT PUSHD_IGNORE_DUPS PUSHD_TO_HOME REC_EXACT
#
##
#
export PATH=$PATH:/usr/games

# .NET Files:
#export DOTNET_ROOT=/usr/share/dotnet
#export MSBuildSDKsPath=$DOTNET_ROOT/sdk/$(${DOTNET_ROOT}/dotnet --version)/Sdks
#export PATH=${PATH}:${DOTNET_ROOT}
#export PATH=$PATH:$HOME/.dotnet/tools
#
# write a file of the current xterm session:
#
#export MANPATH=/usr/local/pgsql/man:$MANPATH
#
#
#POSTSCRIPTLIB=$HOME/lib/postscript
#export POSTSCRIPTLIB
#A2PSHDR=$HOME/lib/a2ps/header.ps
#export A2PSHDR
unset EXINIT
unset NO_CLOBBER
#
#
bindkey "^A"-"^C" self-insert
bindkey "^D" list-choices
bindkey "^E"-"^F" self-insert
bindkey "^G" list-expand
bindkey "^H" vi-backward-delete-char
bindkey "^I" expand-or-complete
bindkey "^J" accept-line
bindkey "^K" self-insert
bindkey "^L" clear-screen
bindkey "^M" accept-line
bindkey "^N"-"^P" self-insert
bindkey "^Q" vi-quoted-insert
bindkey "^R" redisplay
bindkey "^S"-"^T" self-insert
bindkey "^U" vi-kill-line
bindkey "^V" vi-quoted-insert
bindkey "^W" vi-backward-kill-word
bindkey "^X"-"^Z" self-insert
bindkey "^[" vi-cmd-mode
bindkey "^[[5~" up-line-or-history
bindkey "^[[6~" down-line-or-history
bindkey "^[[A" up-line-or-history
bindkey "^[[B" down-line-or-history
bindkey "^[[C" vi-forward-char
bindkey "^[[D" vi-backward-char
bindkey "^\\\\"-"~" self-insert
bindkey "^?" vi-backward-delete-char
bindkey "\M-^@"-"\M-^?" self-insert
#
#
limit coredumpsize 0
export THISSHELL=$$
[ $hasxset -eq 1 ] && xhost +localhost >/dev/null || echo "no xset"

#[ -x /usr/lib/ICAClient/ ] && PATH=/usr/lib/ICAClient:$PATH
#
#export ICAROOT=/usr/lib/ICAClient
env |grep "SSH_CLIENT" 1>/dev/null 2>&1 && {
	remote=`echo $SSH_CLIENT |cut -d" " -f1`	
	PROMPT="%(?..%{[1;31m%}Error: %?
)
%{[1;35m%}%h%{[1;32m%} $Shell-$SessionID %{[1;31m%}ssh from $remote %n@%M:%{[1;33m%}%/%{[1;30m%}%b         
"
	#export DISPLAY=localhost:10
} || true

[ $hasxset -eq 1 ] && xset b off
LS_COLORS='rs=0:di=01;34:ln=01;36:mh=00:pi=40;33:so=01;35:do=01;35:bd=40;33;01:cd=40;33;01:or=40;31;01:mi=00:su=37;41:sg=30;43:ca=30;41:tw=30;42:ow=34;42:st=37;44:ex=01;32:*.tar=01;31:*.tgz=01;31:*.arc=01;31:*.arj=01;31:*.taz=01;31:*.lha=01;31:*.lz4=01;31:*.lzh=01;31:*.lzma=01;31:*.tlz=01;31:*.txz=01;31:*.tzo=01;31:*.t7z=01;31:*.zip=01;31:*.z=01;31:*.Z=01;31:*.dz=01;31:*.gz=01;31:*.lrz=01;31:*.lz=01;31:*.lzo=01;31:*.xz=01;31:*.zst=01;31:*.tzst=01;31:*.bz2=01;31:*.bz=01;31:*.tbz=01;31:*.tbz2=01;31:*.tz=01;31:*.deb=01;31:*.rpm=01;31:*.jar=01;31:*.war=01;31:*.ear=01;31:*.sar=01;31:*.rar=01;31:*.alz=01;31:*.ace=01;31:*.zoo=01;31:*.cpio=01;31:*.7z=01;31:*.rz=01;31:*.cab=01;31:*.wim=01;31:*.swm=01;31:*.dwm=01;31:*.esd=01;31:*.jpg=01;35:*.jpeg=01;35:*.mjpg=01;35:*.mjpeg=01;35:*.gif=01;35:*.bmp=01;35:*.pbm=01;35:*.pgm=01;35:*.ppm=01;35:*.tga=01;35:*.xbm=01;35:*.xpm=01;35:*.tif=01;35:*.tiff=01;35:*.png=01;35:*.svg=01;35:*.svgz=01;35:*.mng=01;35:*.pcx=01;35:*.mov=01;35:*.mpg=01;35:*.mpeg=01;35:*.m2v=01;35:*.mkv=01;35:*.webm=01;35:*.ogm=01;35:*.mp4=01;35:*.m4v=01;35:*.mp4v=01;35:*.vob=01;35:*.qt=01;35:*.nuv=01;35:*.wmv=01;35:*.asf=01;35:*.rm=01;35:*.rmvb=01;35:*.flc=01;35:*.avi=01;35:*.fli=01;35:*.flv=01;35:*.gl=01;35:*.dl=01;35:*.xcf=01;35:*.xwd=01;35:*.yuv=01;35:*.cgm=01;35:*.emf=01;35:*.ogv=01;35:*.ogx=01;35:*.aac=00;36:*.au=00;36:*.flac=00;36:*.m4a=00;36:*.mid=00;36:*.midi=00;36:*.mka=00;36:*.mp3=00;36:*.mpc=00;36:*.ogg=00;36:*.ra=00;36:*.wav=00;36:*.oga=00;36:*.opus=00;36:*.spx=00;36:*.xspf=00;36:';
export LS_COLORS
zstyle ':completion:*' list-colors "${(@s.:.)LS_COLORS}"
autoload -Uz compinit
compinit
#
#
[ "$TERM" = "xterm" -o "$TERM" = "xterm-256color" -o "$TERM" = "rxvt" ] && {
	export _xicon _xtitle _xnames
    # for xterm ESC ] 2 is title of window, 1 is icon name, 0 is both
    xtitle()    { echo -n "\e]2;$*\a";_xtitle=$*     }
    xicon()     { echo -n "\e]1;$*\a";_xicon=$*      }
    xnames()    { xtitle "$*"; xicon "$*";_xnames=$* }
}
xnames "$HOST-$MYSHELL-$SessionID $PWD"
##
# Load host aliases 
hosts
#
# Make cursor orangered:
echo -n -e '\e]12;orangered\a'  

[ -d ~/.localapps ] && {
	for i in ~/.localapps/*
	do 
		. $i
	done
}
