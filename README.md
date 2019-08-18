esp32-peoplecounter

Setup
-----
1. Create device in IoT Central
2. Record Scope ID, Device ID, Primary device key
3. Install DPS Keygen: https://github.com/Azure/dps-keygen
4. Run dps-keygen:
```
npx dps-keygen -di:<device ID> -dk:<device key> -si:<scope ID>
```
5. Record the connection string
6. Copy `include/config.dist.h` to `include/config.h`
7. Edit `include/config.h`, modifiy WiFi settings, and insert the connection string from above

TODO
----
* Currently the people checking loop is interrupted by the Azure upload function. These could easily be run on separate cores to eliminate this issue.

References:
-----------
* https://github.com/PProvost/AzureIoTCentral-Sample-DHT22/blob/master/src/Main.ino
* https://github.com/PProvost/ESP01-AzureIoTCentral-Sample