# SID format

```
  10 9 8 7 6 5 4 3 2 1 0  
  +-+-+-+-+-+-+-+-+-+-+-+
  |*|*| | | | | | | | | |
  +-+-+-+-+-+-+-+-+-+-+-+
   0 0  data
   0 1  management
   1 0  data high priority
   1 1  management high priority 

  10 9 8 7 6 5 4 3 2 1 0  
  +-+-+-+-+-+-+-+-+-+-+-+
  | | |*| | | | | | | | |
  +-+-+-+-+-+-+-+-+-+-+-+
       0 tx(transfer)
       1 rx(receive)

  10 9 8 7 6 5 4 3 2 1 0  
  +-+-+-+-+-+-+-+-+-+-+-+
  | | | |*|*|*|*|*|*|*|*|
  +-+-+-+-+-+-+-+-+-+-+-+
         X X X X X X X X topic
```

## Mapping to MQTT topic


### Network configuration

```
       C A N                         M Q T T
  --+-----+-----+--[can-mqtt gw]--+-----+-----+--
    |     |     |                 |     |     |
   node  node  node              node  node  node
    1     2     3                 a     b     c 
```

### Mapping example

```
"$ node mqtt.js subscribe -t 127/tx" at node-a

"$ node mqtt.js publish -t 127/rx 12345678" at node-a
     10 9 8 7 6 5 4 3 2 1 0  
     +-+-+-+-+-+-+-+-+-+-+-+
SID  |0|0|1|0|1|1|1|1|1|1|1|  and 8bytes data "0x31 0x32 0x33 0x34 0x35 0x36 0x37 0x38"
     +-+-+-+-+-+-+-+-+-+-+-+
```

Masks and filters at node-2
```
     10 9 8 7 6 5 4 3 2 1 0  
     +-+-+-+-+-+-+-+-+-+-+-+
     |*|*|*|*|*|*|*|*|*|*|*|
     +-+-+-+-+-+-+-+-+-+-+-+

RXM0  0 1 1 0 0 0 0 0 0 0 0   (for management)
RXM1  1 1 1 1 1 1 1 1 1 1 1   (for data)

RXF0  0 1 1 0 1 1 1 1 1 1 1   (topic "0x7f/rx" for management)  
RXF1  0 1 0 0 0 0 0 0 0 0 0   (broadcast message for management)
RXF2  0 0 0 0 0 0 0 0 0 0 0 
RXF3  0 0 0 0 0 0 0 0 0 0 0
RXF4  0 0 0 0 0 0 0 0 0 0 0
RXF5  0 0 1 0 1 1 1 1 1 1 1   (topic "0x7F/rx" for data)
```

Masks and filters at can-mqtt gateway 
```
     10 9 8 7 6 5 4 3 2 1 0  
     +-+-+-+-+-+-+-+-+-+-+-+
     |*|*|*|*|*|*|*|*|*|*|*|
     +-+-+-+-+-+-+-+-+-+-+-+

RXM0  0 1 1 0 0 0 0 0 0 0 0   (for management)
RXM1  0 1 1 0 0 0 0 0 0 0 0   (for data)

RXF0  0 1 1 0 0 0 0 0 0 0 0   (topic "0x00/rx" for management)  
RXF1  0 1 0 0 0 0 0 0 0 0 0   (broadcast message for management)
RXF2  0 0 0 0 0 0 0 0 0 0 0
RXF3  0 0 0 0 0 0 0 0 0 0 0
RXF4  0 0 0 0 0 0 0 0 0 0 0
RXF5  0 0 0 0 0 0 0 0 0 0 0   (topic "*/tx" for data)
