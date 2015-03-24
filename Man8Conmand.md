### Name ###

> conmand -- ConMan daemon

### Synopsis ###

> <b>conmand</b> `[`_OPTIONS_`]`

### Description ###

> <b>conmand</b> is the daemon responsible for managing consoles defined
> by its configuration file as well as listening for connections from clients.

### Options ###

> <b>-c</b> _file_
> > Specify a configuration file, overriding the default location
> > `[/etc/conman.conf]`.


> <b>-F</b>
> > Run the daemon in the foreground.


> <b>-h</b>
> > Display a summary of the command-line options.


> <b>-k</b>
> > Send a SIGTERM to the **conmand** process associated with the specified
> > configuration, thereby killing the daemon.  Returns 0 if the daemon
> > was successfully signaled; otherwise, returns 1.


> <b>-L</b>
> > Display license information.


> <b>-p</b> _port_
> > Specify the port on which **conmand** will listen for clients, overriding
> > both the default port `[7890]` and the port specified in the
> > configuration file.


> <b>-q</b>
> > Displays the PID of the **conmand** process associated with the specified
> > configuration if it appears active.  Returns 0 if the configuration
> > appears active; otherwise, returns 1.


> <b>-r</b>
> > Send a SIGHUP to the **conmand** process associated with the specified
> > configuration, thereby re-opening both that daemon's log file and
> > individual console log files.  Returns 0 if the daemon was successfully
> > signaled; otherwise, returns 1.


> <b>-v</b>
> > Enable verbose mode.


> <b>-V</b>
> > Display version information.


> <b>-z</b>
> > Truncate both the daemon's log file and individual console log files
> > at start-up.

### Signals ###


> <b>SIGHUP</b>
> > Close and re-open both the daemon's log file and the individual console
> > log files.  Conversion specifiers within filenames will be re-evaluated.
> > This is useful for **logrotate** configurations.


> <b>SIGTERM</b>
> > Terminate the daemon.

### Security ###


> Connections to the server are not authenticated, and communications between
> client and server are not encrypted.  When time allows, this will be
> addressed in a future release.  Until then, the recommendation is to bind
> the server's listen socket to the loopback address (by specifying
> "**server loopback=on**" in conman.conf) and restrict access to the server
> host.

### Notes ###

> Log messages are sent to standard-error until after the configuration
> file has been read, at which time future messages are discarded unless
> either the **logfile** or **syslog** keyword has been specified (cf.,
> [conman.conf(5)](Man5ConmanConf.md)).

> If the configuration file is modified while the daemon is running and a
> <b>pidfile</b> was not originally specified, the '**-k**' and '**-r**'
> options may be unable to identify the daemon process; consequently,
> the appropriate signal may need to be sent to the daemon manually.

> The number of consoles that can be simultaneously managed is limited
> by the maximum number of file descriptors a process can have open.
> The daemon sets its "nofile" soft limit to the maximum/hard limit.
> If you are encountering "too many open files" errors, you may need to
> increase the "nofile" hard limit.

### Author ###

> Chris Dunlap <cdunlap@llnl.gov>

### Copyright ###

> Copyright (C) 2007-2011 Lawrence Livermore National Security, LLC.<br>
<blockquote>Copyright (C) 2001-2007 The Regents of the University of California.<br></blockquote>

<blockquote>ConMan is free software: you can redistribute it and/or modify it under<br>
the terms of the GNU General Public License as published by the Free<br>
Software Foundation, either version 3 of the License, or (at your option)<br>
any later version.</blockquote>

<h3>See Also</h3>

<blockquote><a href='Man1Conman.md'>conman(1)</a>, <a href='Man5ConmanConf.md'>conman.conf(5)</a>.