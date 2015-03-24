ConMan is a serial console management program designed to support a large number of console devices and simultaneous users.

It supports:

  * local serial devices
  * remote terminal servers (via the telnet protocol)
  * IPMI Serial-Over-LAN (via [FreeIPMI](http://www.gnu.org/software/freeipmi/))
  * Unix domain sockets
  * external processes (e.g., using Expect for telnet / ssh / ipmi-sol connections)

Its features include:

  * logging (and optionally timestamping) console device output to file
  * connecting to consoles in monitor (R/O) or interactive (R/W) mode
  * allowing clients to share or steal console write privileges
  * broadcasting client output to multiple consoles

Documentation:

  * [License Information](LicenseInfo.md)
  * [Manual Pages](ManPages.md)
  * [Release Notes](ReleaseNotes.md)
  * [Tested Platforms](TestedPlatforms.md)