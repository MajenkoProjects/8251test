8251TEST
========

This program is intended to test an 8251 on a CM153 card. It performs
a number of operations depending on the command line switches provided.
Multiple switches will execute multiple operations.

I have no way of testing this program so it may work or it may not. YMMV.

Usage
-----

* Using a different address

You can specify the data and command IO ports for the UART with the switches
/D:... and /C:... such as; 

```
TEST8251 /C:324 /D:323
```

Numbers are hexadecimal and a leading 0x is optional.

There are three operations available at the moment;

```
/L - Perform a loopback test
/B - Perform a baud rate test
/I - Perform a cold init
```

Loopback test
-------------

This test will send the bytes 0-255 in sequence through the UART and expect
to receive the same bytes back in sequence. If nothing is received within
5 seconds a timeout will occur. If the received byte differs from the
transmitted byte a mismatch will occur.

Baud rate test
--------------

This will simply send a constant stream of 'U' characters through the UART.
This has the effect of generating a square wave at half the baud rate.
Press any key to terminate the test.

Cold init
---------

This should only be run once after powerup or a hard reset. It switches the
UART into the initial Async mode. A warm (default) init will use software
to switch back to command mode to set the Async mode.
