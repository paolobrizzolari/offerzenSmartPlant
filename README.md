Example firmware for connecting the ESP8266 over wifi to a mqtt server and having realtime JSON communication bidirectionally.

Edit config. 
Setup your mqtt server on port 1883
Flash.

The device waits for packets like:

```json
{ "data": { "control_pump" : true }}
{ "data": { "control_led" : true }}
```

Recieving either of these will toggle the associated pump or led. The values sent are ignored at this stage.

See https://github.com/OfferZen-Make/plant_tech_ams/blob/master/PREP.md for more.