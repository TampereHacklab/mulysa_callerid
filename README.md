# Mulysa caller ID

esp32 + sim800l hardware for listening for incoming calls and asking mulysa if
the number is allowed or not.

If the number is found to be allowed will pulse pin 23 for a while.

TODO:
Better readme
Pin toggling for relay depending on the numbers access status (how much delay?)
Document hardware and everything else

# Notes

## ring "event"

Incoming call will say something like this

```
RING
+CLIP: "04404XXXXX",129,"",0,"",0
```

BUT only if CLIP is on. to se clip on issue

```
AT+CLIP=1
```

Or check the current status

```
AT+CLIP?
```

## debugging the sim800l
* in loop uncomment loopBackDebug and comment everything else
* open screen with ``` screen /dev/ttyUSB0 115200 ```
* to get echoing say  ``` ATE1 ```
* try out the AT commands


