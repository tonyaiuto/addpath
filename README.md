# addpath
A tool to manipulate PATH-like variables to make writing .bashrc files (and their cousins) easier.

<H3>Synopsis</H3>
<PRE>
addpath [-cbw] [-df] [-e var] [-p var] elem ...
</PRE>
<H3>Description</H3>
<I>addpath</I> appends a set of new paths to your PATH (or other)
environment variable.
It checks for duplicates, so that a new element will not be added if it
is in the PATH already. 
<P>
One might think, "Big whoop, why can't you just get your path right in your
.bashrc file and be done with it?"
Well, I'll tell you. I work in an environment
where I can log into the same home directory from many different platforms,
even (gasp!) non-unix ones, like Windows.
We strive to maintain single file system image accross a ridiculous number of
machines.
With good NFS automounting, many binaries are in the same relative place on
all platforms, but we can't get it right for everything. Worse, we use
multiple versions of various tools on several platforms, so that we
want to constantly switch our paths.
Thus, this need often arises:
<PRE>
	$ echo $PATH
	/users/tony/bin:/usr/local/bin:/bin:/usr/bin:/opt/java/jdk200/bin

		I want to try this with a different JDK

	$ JDK_HOME=/opt/java/jdk117
	$ bash
		I want this to happen
	$ echo $PATH
	/users/tony/bin:/usr/local/bin:/bin:/usr/bin:/opt/java/jdk117/bin
	</PRE>
The practical solution is to do something like this:
<PRE>
	# .bashrc
	PATH=$HOME/bin:/usr/local/bin:/bin:/usr/bin
	if [[ -n "$JDK_HOME" ]] ; then
	    PATH=$PATH:$JDK_HOME/bin
	fi
</PRE>
This works fine up to a point. If you try, howerver, to add something
to your path
by hand, and then invoke the shell again, it gets lost. What you really
want is to be able to set JDK_HOME in the environment and have your shell .rc file
add $JDK_HOME/bin to your path only if it needs it.
With <I>addpath</I> we can do this
<PRE>
	# .bashrc
	# make sure that these are in the path, without
	# disturbing other items
	PATH=$(addpath -f $HOME/bin /usr/local/bin /bin /usr/bin)

	# add the JDK toolkit to the front of the path if we have it
	PATH=$(addpath -f -e JDK_HOME '$JDK_HOME/bin')

	# or use the shorthand. This works because if JDK_HOME is not in
	# the environment, addpath will skip the element which references it
	PATH=$(addpath -f '$JDK_HOME/bin')

	# or use the portable method that works for both Bourne and C shells.
	# (Note that $JDK_HOME must be in single quotes so the shell does not
	# do the evaluation.)
	eval $(addpath -sf '$JDK_HOME/bin')
</PRE>


<H3>Options</H3>
<DL>
<DT>-b</DT>
<DD>emit path in Bourne shell style, using : as a delimiter. (default for most shells)</DD>
<DT>-c</DT>
<DD>emit path in C-sh style. That is, with spaces rather than colons. (default when SHELL ends in "csh"</DD>
<DT>-w</DT>
<DD>emit path in Windows style. That is, semicolon and backslash</DD>
<DT>-d</DT>
<DD>allow Duplicates. Normally <I>addpath</I> will not add elements
which are in the path already</DD>
<DT>-f</DT>
<DD>put new elements at the Front of the PATH</DD>
<DT>-e <I>var</I></DT>
<DD>only do it if Environment variable <I>var</I> is defined.</DD>
<DT>-p <I>var</I></DT>
<DD>Use <I>var</I> rather than PATH.</DD>
<DT>-s</DT>
<DD>emit an output that you could "source" (or eval) back into your shell.
Thus <b>eval $(addpath -s '$ANT_HOME/bin')</b> should work in all shells</DD>
<DT>-x</DT>
<DD>Check new path elements for existance (as directories) before adding.</DD>
</DL>
<H3>Author</H3>
Tony Aiuto  (tony.aiuto@gmail.com)
