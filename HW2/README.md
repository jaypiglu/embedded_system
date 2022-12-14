The socket-server program is able to collect the data from the sensor.
To run this program, you should modify the ssid and password in mbed_app.json, and the socket address in main.cpp and socket-server.ipynb.

![](./resources/official_armmbed_example_badge.png)
# Socket Example

This example shows usage of [network-socket API](https://os.mbed.com/docs/mbed-os/latest/apis/network-socket.html).

The program brings up an underlying network interface and if it's Wifi also scans for access points.
It creates a TCPSocket and performs an HTTP transaction targeting the website in the `mbed_app.json` config.

The example can be configured to use a TLSSocket. This works only on devices that support TRNG.

(Note: To see this example in a rendered form you can import into the Arm Mbed Online Compiler,
please see [the documentation](https://os.mbed.com/docs/mbed-os/latest/apis/socket.html#socket-example).)

## Selecting the network interface

This application is able to use any network interface it finds.

The interface selections is done through weak functions that are overridden by your selected target or any additional
component that provides a network interface.

If more than one interface is provided the target configuration `target.network-default-interface-type`
selects the type provided as the default one. This is usually the Ethernet so building on Ethernet enabled boards,
you do not need any further configuration.

### Configuring mbedtls

By default the examples uses a TCP socket. To enable TLS edit the mbed_app.json to turn on the `use-tls-socket` option:

```
        "use-tls-socket": {
            "value": true
        }
```

It might be necessary to configure the mbedtls library with appropriate macros in mbed_app.json file. Some boards
(like UBLOX_EVK_ODIN_W2) will work fine without any additional configuration and some of them might require some minimal
adjustment. For example K64F requires at least the following macro added:


```
        "K64F": {
            "target.macros_add" : ["MBEDTLS_SHA1_C"]
        }
```

See
[mbedtls configuration guidelines](https://github.com/ARMmbed/mbed-os/tree/master/connectivity/mbedtls#configuring-mbed-tls-features)
for more details.

Also see the API Documentation [TLSSocket](https://os.mbed.com/docs/mbed-os/latest/apis/tlssocket.html).

### WiFi

If you want to use WiFi you need to provide SSID, password and security settings in `mbed_app.json`.

If your board doesn't provide WiFi as the default interface because it has multiple interfaces you need to specify that
you want WiFi in `mbed_app.json`.

```
{
    "target_overrides": {
        "*": {
            "target.network-default-interface-type": "WIFI",
        }
    }
}
```

For more information about Wi-Fi APIs, please visit the
[Mbed OS Wi-Fi](https://os.mbed.com/docs/mbed-os/latest/apis/wi-fi.html)
documentation.

### Supported WiFi hardware

* All Mbed OS boards with build-in Wi-Fi module such as:
    * [ST DISCO IOT board](https://os.mbed.com/platforms/ST-Discovery-L475E-IOT01A/) with integrated
      [ISM43362 WiFi Inventek module](https://github.com/ARMmbed/wifi-ism43362).
    * [ST DISCO_F413ZH board](https://os.mbed.com/platforms/ST-Discovery-F413H/) with integrated
      [ISM43362 WiFi Inventek module](https://github.com/ARMmbed/wifi-ism43362).
* Boards with external WiFi shields such as:
    * [NUCLEO-F429ZI](https://os.mbed.com/platforms/ST-Nucleo-F429ZI/) with ESP8266-01

## Building and flashing the example

### To build the example

Clone the repository containing example:

```
git clone https://github.com/ARMmbed/mbed-os-example-sockets.git
```

**Tip:** If you don't have git installed, you can
[download a zip file](https://github.com/ARMmbed/mbed-os-example-sockets/archive/master.zip)
of the repository.

Update the source tree:

```
cd mbed-os-example-sockets
mbed deploy
```

Run the build:

```mbed compile -t <ARM | GCC_ARM> -m <YOUR_TARGET>```

### To flash the example onto your board

Connect your mbed board to your computer over USB. It appears as removable storage.

When you run the `mbed compile` command above, mbed cli creates a .bin or a .hex file (depending on your target) in
```BUILD/<target-name>/<toolchain>``` under the example's directory. Drag and drop the file to the removable storage.

Alternatively you may launch compilation with `-f` flag to have mbed tools attempt to flash your board.
The tools will flash the binary to all targets that match the board specified by '-m' parameter. 

## Running the example


When example application is running information about activity is printed over the serial connection.

**Note:** The default serial baudrate has been set to 9600.

Please have a client open and connected to the board. You may use:

- [Tera Term](https://ttssh2.osdn.jp/index.html.en) for windows

- screen or minicom for Linux (example usage: `screen /dev/serial/<your board> 9600`)

- mbed tools has a terminal command `mbed term -b 9600`

### Expected output

(Assuming you are using a wifi interface, otherwise the scanning will be skipped)

```
Starting socket demo

2 networks available:
Network: Virgin Media secured: Unknown BSSID: 2A:35:D1:ba:c7:41 RSSI: -79 Ch: 6
Network: VM4392164 secured: WPA2 BSSID: 18:35:D1:ba:c7:41 RSSI: -79 Ch: 6

Connecting to the network...
IP address: 192.168.0.27
Netmask: 255.255.255.0
Gateway: 192.168.0.1

Resolve hostname ifconfig.io
ifconfig.io address is 104.24.122.146

sent 52 bytes: 
GET / HTTP/1.1
Host: ifconfig.io
Connection: close

received 256 bytes:
HTTP/1.1 200 OK

Demo concluded successfully 
```

## License and contributions

The software is provided under Apache-2.0 license. Contributions to this project are accepted under the same license.
Please see [contributing.md](CONTRIBUTING.md) for more info.

This project contains code from other projects. The original license text is included in those source files.
They must comply with our license guide
