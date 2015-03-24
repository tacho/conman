### [conman-0.2.7](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.7.tar.bz2) (2011-04-21) ###

  * Fixed concurrency bugs in IPMI SOL connections. ([issue 7](https://code.google.com/p/conman/issues/detail?id=7))
  * Added support for IPMI SOL workaround flags and whatnot. ([issue 5](https://code.google.com/p/conman/issues/detail?id=5))
  * Added support for disabling fcntl locks on console logs. ([issue 6](https://code.google.com/p/conman/issues/detail?id=6))
  * Note: "--with-freeipmi" now requires freeipmi-1.0.4 or later.

### [conman-0.2.6](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.6.tar.bz2) (2010-06-15) ###

  * Changed license to GPLv3+.
  * Changed project homepage to `<http://conman.googlecode.com/>`.
  * Fixed bugs with IPMI SOL reconnects. ([issue 1](https://code.google.com/p/conman/issues/detail?id=1)) ([issue 2](https://code.google.com/p/conman/issues/detail?id=2))
  * Fixed bug with process-driven reconnects. ([issue 3](https://code.google.com/p/conman/issues/detail?id=3))

### [conman-0.2.5](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.5.tar.bz2) (2009-05-19) ###

  * Fixed bug with consoles trapped in downed state causing poll loop to spin.
  * Added "server coredump" & "server coredumpdir" config directives.

### [conman-0.2.4.1](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.4.1.tar.bz2) (2009-04-21) ###

  * Fixed builds with gcc-4.4 on Fedora 11.

### [conman-0.2.4](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.4.tar.bz2) (2009-04-14) ###

  * Added support for FreeIPMI.
  * Added console scripts for Sun ELOM and Sun Fire V20z / V40z.
  * Changed conmen cmdline opts: "-G" xterm geometry, "-g" genders nodeattr.

### [conman-0.2.3](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.3.tar.bz2) (2008-12-03) ###

  * Added support for creation of intermediate directories.
  * Added console script for HP LO100.

### [conman-0.2.2](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.2.tar.bz2) (2008-09-03) ###

  * Added support for connecting to consoles via Unix domain sockets.
  * Added support for detecting Unix domain socket devices via inotify.
  * Added client escape character to toggle echoing of client input via "&E".
  * Added console script for HP iLO.
  * Updated console script for IBM BladeCenter.

### [conman-0.2.1](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.1.tar.bz2) (2007-05-24) ###

  * Added console script for Sun ILOM.
  * Fixed minor issues with existing console scripts.

### [conman-0.2.0](http://code.google.com/p/conman/downloads/detail?name=conman-0.2.0.tar.bz2) (2007-05-02) ###

  * Changed project homepage to `<http://home.gna.org/conman/>`.
  * Added support for consoles to be driven by an external process.
  * Added console scripts for Cyclades, IBM BladeCenter, IPMI SOL, SSH.
  * Added "server execpath" config directive.
  * Added "-F" cmdline opt to daemon for running in the foreground.
  * Changed daemon behavior to return fatal errors to shell if possible.
  * Sanitized environment variables and closed unneeded fds in child processes.
  * Fixed potential deadlock bug in signal handlers.

### [conman-0.1.9.2](http://code.google.com/p/conman/downloads/detail?name=conman-0.1.9.2.tar.bz2) (2006-06-26) ###

  * Fixed bug where a failed telnet connection would segfault the daemon.
  * Optimized main I/O polling loop.

### conman-0.1.9.1 (2006-05-25) ###

  * Fixed rpm spec file to include sysconfig / defaults file.

### conman-0.1.9 (2006-05-25) ###

  * Added support for >1024 (`FD_SETSIZE`) file descriptors.
  * Increased limit for number of open files.
  * Decreased startup time by moving hostname resolution to connection attempt.
  * Added support to retry connection attempt if hostname resolution fails.
  * Replaced "--with-conman-conf" configure opt with use of "--sysconfdir".
  * Enabled tcp-wrappers support by default if possible.
  * Fixed potential NULL dereference and resource leaks.
  * Rewrote init script.

### [conman-0.1.8.8](http://code.google.com/p/conman/downloads/detail?name=conman-0.1.8.8.tar.bz2) (2003-10-07) ###

  * Changed behavior of timestamp logopt.

### conman-0.1.8.7 (2003-10-01) ###

  * Added timestamp logopt to enable timestamping each line of console log.
  * Changed sanitize logopt to ignore NULs on newline transition.

### conman-0.1.8.6 (2003-07-24) ###

  * Added fallback for determining daemon's pid via use of pidfile.
  * Added "-q" cmdline opt to daemon to query daemon's pid.
  * Decreased startup time.
  * Rewrote init script.

### conman-0.1.8.5 (2003-07-13) ###

  * Added support for "%" conversion specifiers.
  * Deprecated former "&" specifier.

### conman-0.1.8.4 (2003-04-03) ###

  * Fixed init script to properly name RedHat's subsys file lock.

### conman-0.1.8.3 (2002-09-04) ###

  * Increased init script ulimit to allow >1024 file descriptors.

### conman-0.1.8.2 (2002-08-13) ###

  * Added client escape character for sending raw DEL char via "&D".

### conman-0.1.8.1 (2002-07-15) ###

  * Added telnet connection fix for ECHO and SUPPRESS GO AHEAD requests.

### [conman-0.1.8](http://code.google.com/p/conman/downloads/detail?name=conman-0.1.8.tar.bz2) (2002-05-20) ###

  * Added "global log" config directive.
  * Added "server logfile" & "server syslog" config directives.
  * Added "-L" cmdline opt for software license.
  * Fixed bug where relative-path'd pidfile was not removed at exit.
  * Fixed bug where relative-path'd logs were not re-opened on sighup.
  * Fixed bug where empty strings were not detected during config parsing.
  * Fixed bug where pidfile write was still attempted if its open had failed.
  * Updated manpages.