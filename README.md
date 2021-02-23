Example firmware for connecting the ESP8266 over wifi to a mqtt server and having realtime JSON communication bidirectionally.

Edit config. 
Setup your mqtt server on port 1883
Flash.

The device waits for packets like:

```json
{ data: { control_pump : any }}
{ data: { control_led : any }}
```

Recieving either of these will toggle the associated pump or led.

See https://github.com/OfferZen-Make/plant_tech_ams/blob/master/PREP.md for more.