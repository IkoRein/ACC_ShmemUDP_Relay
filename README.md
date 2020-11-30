# ACC_ShmemUDP_Relay

Tool to send the Shared Memory contents from Assetto Corsa Competizione (ACC) via UDP

The code is based on Kunos Simulazioni & Enzi Ferrum work on the ACC Shared Memory Example. 
The specs and the original code is at 
https://www.assettocorsa.net/forum/index.php?threads/acc-shared-memory-documentation.59965/

This little piece of code is used by my Telemetry tool to receive data from the ACC. The actual
Telemetry tool for racing simulators is at 
https://www.racedepartment.com/downloads/telemetry-application-v10.34318/ 

I have
- added UDP networking, so the data can be send via UDP
- changed key input reading to not trigger AV warnings in compiled code
- added to the UDP packets extra bit to indicate, which packet is sent
- added RelayVersion struct to notify via UDP the version of the UDP_Relay
- cleaned the code a bit and added some comments


I have shared this, so others can also benefit from the code.

I plan to update this code if there is change in the Shared Memory specs or if I add some code,
my Telemetry tool needs.





