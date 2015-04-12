#!/bin/sh

ADDPATH=../addpath
PATH=/usr/bin:/usr/local/bin
CPATH="/usr/bin /usr/local/bin"
HOME=/home/tony
export HOME
APP_HOME=/usr/share/myapp
export APP_HOME
MAN_PATH=/usr/share/man
export MAN_PATH
VERBOSE=1

checkit()
{
	name=$1
	want=$2
	shift
	shift
	P=`$ADDPATH $*`
	if [ "$P" = "$want" ] ; then
		if [ $VERBOSE -gt 0 ] ; then
			echo PASSED: "$name"
		fi
	else
		if [ $VERBOSE -gt 0 ] ; then
			echo 'FAILED: '"$name": expect "$want"
			echo '        '"$name"'     got '"$P"
		fi
	fi
}

PATH=/usr/bin:/usr/local/bin
SHELL=/bin/bash
export SHELL
checkit simple /usr/bin:/usr/local/bin:/opt/New:/usr/X11R6/bin \
		/usr/bin /opt/New /usr/X11R6/bin

checkit prepend /opt/New:/usr/X11R6/bin:/usr/bin:/usr/local/bin \
		-f /usr/bin /opt/New /usr/X11R6/bin

checkit csh_simple '/usr/bin /usr/local/bin /opt/New /usr/X11R6/bin' \
		-c /usr/bin /opt/New /usr/X11R6/bin
checkit csh_prepend '/opt/New /usr/X11R6/bin /usr/bin /usr/local/bin' \
		-fc /usr/bin /opt/New /usr/X11R6/bin

checkit envtest $HOME/bin:$PATH  \
		-f -e HOME '$HOME/bin'

checkit csh_envtest "$HOME/bin $CPATH" \
		-fc -e HOME '$HOME/bin'

checkit csh_env2 "$CPATH" \
	 -fc -e FLOAM /home/tony/bin

XPATH=/foo/bar/baz
export XPATH
DXPATH='\foo\bar\baz'

checkit pflag "$XPATH:znorp/boop" \
	 -p XPATH znorp/boop

checkit dos_pflag "$DXPATH;znorp\\boop" \
	 -w -p XPATH znorp/boop

checkit noenv "$PATH" \
	 -e NOENV '$APP_HOME/bin'

checkit csh_noenv "$CPATH" \
	 -c -e NOENV 'will_not_add'

checkit autoenvcheck "$PATH:$HOME/frobnitz" \
	 '$HOME/frobnitz'

checkit autoenv2 "$PATH" \
	 '$NOVAR/frobnitz'

checkit dup1 "$PATH:/usr/bin" \
	 -d /usr/bin

checkit dup2 "$PATH:/usr/share/myapp/bin" \
	 -d '$APP_HOME/bin'

checkit dupcsh "$CPATH $APP_HOME/bin" \
	 -cd '$APP_HOME/bin'

checkit dupfront "/home/tony/bin:$PATH" \
	 -fd $HOME/bin

testvar=/bin:/usr/bin::/usr/local/bin
export testvar
checkit dupcolon "$testvar:/home/bin" \
	-b -p testvar /home/bin

checkit prefixbug "$PATH:/usr/binx" -b /usr/binx

checkit source "PATH=$PATH:/foo/bar" -s /foo/bar

checkit sourcep "MAN_PATH=$MAN_PATH:/foo/bar" -p MAN_PATH -s /foo/bar

checkit xoption1 "$PATH:/etc" -bx /etc

checkit xoption2 "$PATH" -bx /foo/bar

#######   C-sh auto detecting tests past this point ########

SHELL=/bin/csh
checkit autocsh "/usr/bin /usr/local/bin /opt/New /usr/X11R6/bin" \
		/usr/bin /opt/New /usr/X11R6/bin

checkit sourcecsh 'set path = ( /usr/bin /usr/local/bin /foo/bar )' -s /foo/bar



exit 0
