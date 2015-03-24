### Name ###

> conman -- ConMan client

### Synopsis ###

> <b>conman</b> `[`_OPTIONS_`]` ... `[`_CONSOLES_`]` ...

### Description ###

> <b>conman</b> is a program for connecting to remote consoles being
> managed by **conmand**.  Console names can be separated by spaces and/or
> commas.  Globbing is used by default to match console names against the
> configuration, but regular expression matching can be enabled with the
> '**-r**' option.

> <b>conman</b> supports three modes of console access: monitor (read-only),
> interactive (read-write), and broadcast (write-only).  If neither the
> '**-m**' (monitor) nor '**-b**' (broadcast) options are specified, the console
> session is opened in interactive mode.

### Options ###

> <b>-b</b>
> > Broadcast to multiple consoles (write-only).  Data sent by the client
> > will be copied to all specified consoles in parallel, but console
> > output will not be sent back to the client.  This option can be used
> > in conjunction with '**-f**' or '**-j**'.


> <b>-d</b> _destination_
> > Specify the location of the **conmand** daemon, overriding the default
> > `[127.0.0.1:7890]`.  This location may contain a hostname or IP address,
> > and be optionally followed by a colon and port number.


> <b>-e</b> _character_
> > Specify the client escape character, overriding the default `[`**&**`]`.


> <b>-f</b>
> > Specify that write-access to the console should be "forced", thereby
> > stealing the console away from existing clients having write privileges.
> > The original clients are informed by **conmand** of who perpetrated the
> > theft as their connections are terminated.


> <b>-F</b> _file_
> > Read console names/patterns from file.  Only one console name may be
> > specified per line.  Leading and trailing whitespace, blank lines,
> > and comments (i.e., lines beginning with a '#') are ignored.


> <b>-h</b>
> > Display a summary of the command-line options.


> <b>-j</b>
> > Specify that write-access to the console should be "joined", thereby
> > sharing the console with existing clients having write privileges.
> > The original clients are informed by **conmand** that a new client has
> > been granted write privileges.


> <b>-l</b> _file_
> > Log console session output to file.


> <b>-L</b>
> > Display license information.


> <b>-m</b>
> > Monitor a console (read-only).


> <b>-q</b>
> > Query **conmand** for consoles matching the specified names/patterns.
> > Output from this query can be saved to file for use with the '**-F**'
> > option.


> <b>-Q</b>
> > Enable quiet-mode, suppressing informational messages.  This mode can
> > be toggled within a console session via the '**&Q**' escape.


> <b>-r</b>
> > Match console names via regular expressions instead of globbing.


> <b>-v</b>
> > Enable verbose mode.


> <b>-V</b>
> > Display version information.

### Escape Characters ###


> The following escapes are supported and assume the default escape character
> `[`**&**`]`:

> <b>&?</b>
> > Display a list of currently available escapes.


> <b>&.</b>
> > Terminate the connection.


> <b>&&</b>
> > Send a single escape character.


> <b>&B</b>
> > Send a "serial-break" to the remote console.


> <b>&E</b>
> > Toggle echoing of client input.


> <b>&F</b>
> > Switch from read-only to read-write via a "force".


> <b>&I</b>
> > Display information about the connection.


> <b>&J</b>
> > Switch from read-only to read-write via a "join".


> <b>&L</b>
> > Replay up the the last 4KB of console output.  This escape requires
> > the console device to have logging enabled in the **conmand** configuration.


> <b>&M</b>
> > Switch from read-write to read-only.


> <b>&Q</b>
> > Toggle quiet-mode to display/suppress informational messages.


> <b>&R</b>
> > Reset the node associated with this console.  This escape requires a
> > "resetcmd" to be specified in the **conmand** configuration.


> <b>&Z</b>
> > Suspend the client.

### Environment ###


> The following environment variables override the default settings.

> <b>CONMAN_HOST</b>
> > Specifies the hostname or IP address at which to contact **conmand**,
> > but may be overridden by the '**-d**' command-line option.  A port number
> > separated by a colon may follow the hostname (i.e., _host_:_port_),
> > although the `CONMAN_PORT` environment variable takes precedence.
> > If not set, the default host `[127.0.0.1]` will be used.


> <b>CONMAN_PORT</b>
> > Specifies the port on which to contact **conmand**, but may be overridden
> > by the '**-d**' command-line option.  If not set, the default port
> > `[7890]` will be used.


> <b>CONMAN_ESCAPE</b>
> > The first character of this variable specifies the escape character,
> > but may be overridden by the '**-e**' command-line option.  If not set,
> > the default escape character `[`**&**`]` will be used.

### Security ###


> The client/server communications are not yet encrypted.

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

<blockquote><a href='Man5ConmanConf.md'>conman.conf(5)</a>, <a href='Man8Conmand.md'>conmand(8)</a>.