# Use Mac as the Client

Compile the client on the machine that will run it. If you use a Mac to connect
to the board over serial and Ethernet, run these commands on your Mac from the
`project` directory:

```sh
make client_mac
./client_mac -i 10.10.7.1 -f LittlePrince.txt
```

The executable is named `client_mac` to avoid a collision between `client` and
the `Client` directory on case-insensitive macOS filesystems. On Linux, use
`make client` to build the `client` executable.

`make clean` removes both client executable names along with the other generated files.
