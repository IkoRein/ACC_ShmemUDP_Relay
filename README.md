# ACC_ShmemUDP_Relay

Tool to send the Shared Memory contents from Assetto Corsa Competizione (ACC) via UDP

The code is based on Kunos Simulazioni & Enzi Ferrum work on the ACC Shared Memory Example. 
The specs and the original code is at 
https://www.assettocorsa.net/forum/index.php?threads/acc-shared-memory-documentation.59965/

This little piece of code is used by my Telemetry tool to receive data from the ACC. The actual
Telemetry tool for racing simulators is at 
https://www.racedepartment.com/downloads/telemetry-application-v10.34318/ 

I have
- Added UDP functionality to send the Shared Memory structs via UDP
- Changed/fixed key input reading to avoid AV flagging of the compiled code
- Added 1 byte to the start of each packet to indicate the type of packet
- Some cleaning + added some comments

I have shared this, so others can also benefit from the code.

I plan to update this code if there is change in the Shared Memory specs or if I add some code,
my Telemetry tool needs.





