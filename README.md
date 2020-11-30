# ACC_ShmemUDP_Relay

Tool to send the Shared Memory contents from Assetto Corsa Competizione (ACC) via UDP

This code is based on Kunos Simulation and Ensi Ferrum's Assetto Corsa Competizione shared memory example at
https://www.assettocorsa.net/forum/index.php?threads/acc-shared-memory-documentation.59965/

I have:
- added UDP networking, so the data can be send via UDP
- changed key input reading to not trigger AV warnings in compiled code
- added to the UDP packets extra bit to indicate, which packet is sent
- added RelayVersion struct to notify via UDP the version of the UDP_Relay
- cleaned the code a bit and added some comments

Posted this here, so others can also take advantage of this.

If there is some super serious bug or new version of the SHMEM spec, I will update this code.
