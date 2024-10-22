# Use Mac as the Client
We have a specific client source file and a rule, 'make client_mac', for people who use Mac as the runtime machine. The name of the executable file is changed to 'client_mac' instead of 'client' because it will overwrite the 'Client' folder if they are in the same directory.  

client.cpp should be compiled on the runtime machine. That means if you are using Mac to connect to the board with serial and Ethernet, you should run 'make client_mac' on your Mac and the rest of 'make' commands on the biglab machine. 