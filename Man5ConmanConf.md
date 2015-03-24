### Name ###

> conman.conf -- ConMan daemon configuration file

### Description ###

> The **conman.conf** configuration file is used to specify the consoles
> being managed by **conmand**.

> Comments are introduced by a hash sign (#), and continue until the end
> of the line.  Blank lines and white-space are ignored.  Directives are
> terminated by a newline, but may span multiple lines by escaping it (i.e.,
> immediately preceding the newline with a backslash).  Strings may be
> either single-quoted or double-quoted, but they may not contain newlines.
> Keywords are case-insensitive.

### Server Directives ###

> These directives begin with the **server** keyword followed by one of the
> following key/value pairs:

> <b>coredump</b> = ( **on** | **off** )
> > Specifies whether the daemon should generate a core dump file.  This file
> > will be created in the current working directory (or '/' when running in
> > the background) unless you also set **coredumpdir**.  The default is **off**.


> <b>coredumpdir</b> = "_directory_"
> > Specifies the directory where the daemon tries to write core dump files.
> > The default is empty, meaning the current working directory (or '/'
> > when running in the background) will be used.


> <b>execpath</b> = "_dir1:dir2:dir3..._"
> > Specifies a colon-separated list of directories in which to search for
> > external process-based console executables that are not defined by an
> > absolute or relative pathname.  The default is empty.


> <b>keepalive</b> = ( **on** | **off** )
> > Specifies whether the daemon will use TCP keep-alives for detecting
> > dead connections.  The default is **on**.


> <b>logdir</b> = "_directory_"
> > Specifies a directory prefix for log files that are not defined via an
> > absolute pathname.  This affects the **server logfile**, **global log**,
> > and **console log** directives.


> <b>logfile</b> = "_file_`[`,_priority_`]`"
> > Specifies the file to which log messages are appended if the daemon is
> > not running in the foreground.  This string undergoes conversion specifier
> > expansion (cf., [Conversion Specifications](#Conversion_Specifications.md))
> > each time the file is opened.  If an absolute pathname is not given,
> > the file's location is relative to either **logdir** (if defined) or the
> > current working directory.  Intermediate directories will be created as
> > needed.  The filename may optionally be followed by a comma and a minimum
> > priority at which messages will be logged.  Refer to **syslog.conf(5)**
> > for a list of priorities.  The default priority is **info**.  If this
> > keyword is used in conjunction with the **syslog** keyword, messages will
> > be sent to both locations.


> <b>loopback</b> = ( **on** | **off** )
> > Specifies whether the daemon will bind its socket to the loopback address,
> > thereby only accepting local client connections directed to that address
> > (`127.0.0.1`).  The default is **off**.


> <b>pidfile</b> = "_file_"
> > Specifies the file to which the daemon's PID is written.  Intermediate
> > directories will be created as needed.  The use of a pidfile is
> > recommended if you want to use the daemon's '**-k**', '**-q**', or '**-r**'
> > options.


> <b>port</b> = _integer_
> > Specifies the port on which the daemon will listen for client connections.


> <b>resetcmd</b> = "_string_"
> > Specifies a command string to be invoked by a subshell upon receipt of
> > the client's "reset" escape.  Multiple commands within a string may be
> > separated with semicolons.  This string undergoes conversion specifier
> > expansion (cf., [Conversion Specifications](#Conversion_Specifications.md))
> > and will be invoked multiple times if the client is connected to
> > multiple consoles.


> <b>syslog</b> = "_facility_"
> > Specifies that log messages are to be sent to the system logger
> > (**syslogd**) at the given facility.  Refer to **syslog.conf(5)** for a
> > list of facilities.  If this keyword is used in conjunction with
> > the **logfile** keyword, messages will be sent to both locations.


> <b>tcpwrappers</b> = ( **on** | **off** )
> > Specifies whether the daemon will use TCP-Wrappers when accepting
> > client connections.  Support for this feature must be enabled
> > at compile-time (via configure's "`--with-tcp-wrappers`" option).
> > Refer to **hosts\_access(5)** and **hosts\_options(5)** for more details.
> > The default is **off**.


> <b>timestamp</b> = _integer_ ( **m** | **h** | **d** )
> > Specifies the interval between timestamps written to the individual
> > console log files.  The interval is an integer that may be followed
> > by a single-character modifier; '**m**' for minutes (the default), '**h**'
> > for hours, or '**d**' for days.  The default is 0 (i.e., no timestamps).

### Global Directives ###


> These directives begin with the **global** keyword followed by one of the
> following key/value pairs:

> <b>log</b> = "_file_"
> > Specifies the default log file to use for each **console** directive.
> > This string undergoes conversion specifier expansion (cf.,
> > [Conversion Specifications](#Conversion_Specifications.md)) each time the
> > file is opened; it must contain either '**%N**' or '**%D**'.  If an absolute
> > pathname is not given, the file's location is relative to either **logdir**
> > (if defined) or the current working directory.  Intermediate directories
> > will be created as needed.


> <b>logopts</b> = "( **lock** | **nolock** ),( **sanitize** | **nosanitize** ),( **timestamp** | **notimestamp** )"

<blockquote>
<blockquote>Specifies global options for the console log files.  These options can<br>
be overridden on a per-console basis by specifying the <b>console logopts</b>
keyword.  Note that options affecting the output of the console's<br>
logfile also affect the output of the console's log-replay escape.<br>
The valid <b>logopts</b> include the following:</blockquote>

<blockquote><b>lock</b> or <b>nolock</b> - locked logs are protected with a write lock.</blockquote>

<blockquote><b>sanitize</b> or <b>nosanitize</b> - sanitized logs convert non-printable<br>
characters into 7-bit printable characters.</blockquote>

<blockquote><b>timestamp</b> or <b>notimestamp</b> - timestamped logs prepend each line<br>
of console output with a timestamp in "YYYY-MM-DD HH:MM:SS" format.<br>
This timestamp is generated when the first character following the line<br>
break is output.</blockquote>

<blockquote>The default is "<b>lock</b>,<b>nosanitize</b>,<b>notimestamp</b>".<br>
</blockquote></blockquote>

> <b>seropts</b> = "_bps_`[`,_databits_`[`_parity_`[`_stopbits_`]``]``]`"

<blockquote>
<blockquote>Specifies global options for local serial devices.  These options can<br>
be overridden on a per-console basis by specifying the <b>console seropts</b>
keyword.</blockquote>

<blockquote><i>bps</i> is an integer specifying the baud rate in bits-per-second.<br>
If this exact value is not supported by the system, it will be rounded<br>
down to the next supported value.</blockquote>

<blockquote><i>databits</i> is an integer from 5-8.</blockquote>

<blockquote><i>parity</i> is a single case-insensitive character: '<b>n</b>' for none, '<b>o</b>'<br>
for odd, and '<b>e</b>' for even.</blockquote>

<blockquote><i>stopbits</i> is an integer from 1-2.</blockquote>

<blockquote>The default is "<code>9600,8n1</code>" for 9600 bps, 8 data bits, no parity,<br>
and 1 stop bit.<br>
</blockquote></blockquote>

> <b>ipmiopts</b> = "**U**:_str_,**P**:_str_,**K**:_str_,**C**:_int_,**L**:_str_,**W**:_flag_"

<blockquote>
<blockquote>Specifies global options for IPMI Serial-Over-LAN devices.<br>
These options can be overridden on a per-console basis by specifying<br>
the <b>console ipmiopts</b> keyword.  This directive is only available if<br>
configured using the "<code>--with-freeipmi</code>" option.</blockquote>

<blockquote>The <b>ipmiopts</b> string is parsed into comma-delimited substrings where each<br>
substring is of the form "<i>X</i>:<i>VALUE</i>".  "<i>X</i>" is a single-character<br>
case-insensitive key specifying the option type, and "<i>VALUE</i>" is<br>
its corresponding value.  The IPMI default will be used if either<br>
"<i>VALUE</i>" is omitted from the substring ("<i>X</i>:") or the substring is<br>
omitted altogether.  Note that since the <b>ipmiopts</b> string is delimited<br>
by commas, substring values cannot contain commas.</blockquote>

<blockquote>The valid <b>ipmiopts</b> substrings include the following (in any order):</blockquote>

<blockquote><b>U</b>:<i>username</i> - a string of at most 16 bytes for the username.</blockquote>

<blockquote><b>P</b>:<i>password</i> - a string of at most 20 bytes for the password.</blockquote>

<blockquote><b>K</b>:<i>K_g</i> - a string of at most 20 bytes for the K_g key.</blockquote>

<blockquote><b>C</b>:<i>cipher_suite</i> - an integer for the IPMI cipher suite ID.<br>
Refer to <a href='http://www.gnu.org/software/freeipmi/manpages/man8/ipmiconsole.8.html'>ipmiconsole(8)</a>
for a list of supported IDs.</blockquote>

<blockquote><b>L</b>:<i>privilege_level</i> - the string "user", "op", or "admin".</blockquote>

<blockquote><b>W</b>:<i>workaround_flag</i> - a string or integer for an IPMI workaround.<br>
The following strings are recognized: "authcap", "intel20", "supermicro20",<br>
"sun20", "opensesspriv", "integritycheckvalue", "solpayloadsize",<br>
"solport", and "solstatus".<br>
Refer to <a href='http://www.gnu.org/software/freeipmi/manpages/man8/ipmiconsole.8.html'>ipmiconsole(8)</a>
for more information on these workarounds.  This substring may be repeated<br>
in order to specify multiple workarounds.</blockquote>

<blockquote>Both the <i>password</i> and <i>K_g</i> values can be specified in either ASCII or<br>
hexadecimal; in the latter case, the string should begin with "0x" and<br>
contain at most 40 hexadecimal digits.  A <i>K_g</i> key entered in hexadecimal<br>
may contain embedded null characters, but any characters following the<br>
first null character in the <i>password</i> key will be ignored.<br>
</blockquote></blockquote>

### Console Directives ###

> This directive defines an individual console being managed by the daemon.
> The **console** keyword is followed by one or more of the following
> key/value pairs:

> <b>name</b> = "_string_"
> > Specifies the name used by clients to refer to the console.  This keyword
> > is required.


> <b>dev</b> = "_string_"

<blockquote>
<blockquote>Specifies the type and location of the device.  This keyword is required.</blockquote>

<blockquote>A local serial port connection is defined by the pathname of the<br>
character device file.</blockquote>

<blockquote>A remote terminal server connection using the telnet protocol is defined<br>
by the "<i>host</i>:<i>port</i>" format (where <i>host</i> is the remote hostname or<br>
IPv4 address, and <i>port</i> is the remote port number).</blockquote>

<blockquote>An external process-based connection is defined by the "<i>path args</i>"<br>
format (where <i>path</i> is the pathname to an executable file/script,<br>
and any additional <i>args</i> are space-delimited).</blockquote>

<blockquote>A local Unix domain socket connection is defined by the "<code>unix:</code><i>path</i>"<br>
format (where "<code>unix:</code>" is the literal character string prefix and <i>path</i>
is the pathname of the local socket).</blockquote>

<blockquote>An IPMI Serial-Over-LAN connection is defined by the "<code>ipmi:</code><i>host</i>"<br>
format (where "<code>ipmi:</code>" is the literal string and <i>host</i> is a hostname<br>
or IPv4 address).</blockquote>

<blockquote>The '<b>%N</b>' character sequence will be replaced by the console name.<br>
</blockquote></blockquote>

> <b>log</b> = "_file_"
> > Specifies the file where console output is logged.  This string
> > undergoes conversion specifier expansion (cf.,
> > [Conversion Specifications](#Conversion_Specifications.md)) each time the file
> > is opened.  If an absolute pathname is not given, the file's location is
> > relative to either **logdir** (if defined) or the current working directory.
> > Intermediate directories will be created as needed.  An empty log string
> > (i.e., **log**="") disables logging, overriding the **global log** name.


> <b>logopts</b> = "_string_"
> > This keyword is optional (cf., [Global Directives](#Global_Directives.md)).


> <b>seropts</b> = "_string_"
> > This keyword is optional (cf., [Global Directives](#Global_Directives.md)).


> <b>ipmiopts</b> = "_string_"
> > This keyword is optional (cf., [Global Directives](#Global_Directives.md)).

### Conversion Specifications ###


> A conversion specifier is a two-character sequence beginning with a '**%**'
> character.  The second character in the sequence specifies the type of
> conversion to be applied.  The following specifiers are supported:

> <b>%N</b>
> > The console name (from the **name** string).


> <b>%D</b>
> > The console device basename (from the **dev** string), with leading
> > directory components removed.


> <b>%P</b>
> > The daemon's process identifier.


> <b>%Y</b>
> > The year as a 4-digit number with the century.


> <b>%y</b>
> > The year as a 2-digit number without the century.


> <b>%m</b>
> > The month as a 2-digit number (01-12).


> <b>%d</b>
> > The day of the month as a 2-digit number (01-31).


> <b>%H</b>
> > The hour as a 2-digit number using a 24-hour clock (00-23).


> <b>%M</b>
> > The minute as a 2-digit number (00-59).


> <b>%S</b>
> > The seconds as a 2-digit number (00-61).


> <b>%s</b>
> > The number of seconds since the Epoch.


> <b>%%</b>
> > A literal '%' character.


> The console name (**%N**) and device (**%D**) specifiers are "sanitized"
> in that non-printable characters and the forward-slash (/) character are
> replaced with underscores.

> Conversion specifiers within console log filenames are evaluated when the
> file is opened; this occurs when **conmand** first starts and whenever it
> receives a **SIGHUP**.

### Files ###

> _/etc/conman.conf_

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

<blockquote><a href='Man1Conman.md'>conman(1)</a>, <a href='Man8Conmand.md'>conmand(8)</a>.