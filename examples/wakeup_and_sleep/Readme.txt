
This is a very simple example of how to use the rfm12 lib.

It only sends packets periodically, while receiving packets at the same time.
Whenever a packet is transmitted, a dot '.' is printed on the uart.
Whenever a packet is received, it's content is printed on the uart as ascii,
and the state of the LED is toggled.

Set up your Hardware configuration and uart baudrate in ../config.h !

When you run two Units with this firmware at once, you should see the dots
and the received message "foobar" on both uarts, and in case you haven't
connected the uart, you should atleast be able to see the LEDs blinking,
whenever the two units receive each others packets.
