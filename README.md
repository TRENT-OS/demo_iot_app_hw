# Demo IoT App RPi3

Demo application that showcases a rather typical IoT use case. A Sensor
component cyclically publishes a temperature update via MQTT to a
CloudConnector component. This component then opens up a TLS-Connection to an
MQTT broker and in turn publishes the MQTT message to it. The configuration for
the NetworkStack, the Sensor and the CloudConnector component can be
set in the "config.xml" file. When running the demo, the
NetworkStack, Sensor and CloudConnector component will all contact the ConfigServer
and retrieve their required configuration parameters through it. Once all
components have initialized, the Sensor will proceed to contact the
CloudConnector about every 5 seconds to send a message to the configured broker.

0. Create the application image `os_image.elf`:

```bash
seos_tests/seos_sandbox/scripts/open_trentos_build_env.sh seos_tests/seos_sandbox/build-system.sh seos_tests/src/demos/demo_iot_app_rpi3 rpi3 build-rpi3-Debug-demo_iot_app_rpi3 -DCMAKE_BUILD_TYPE=Debug
```

```bash
seos_tests/seos_sandbox/scripts/open_trentos_build_env.sh seos_tests/seos_sandbox/build-system.sh seos_tests/src/demos/demo_iot_app_rpi3 rpi4 build-rpi4-Debug-demo_iot_app_rpi3 -DCMAKE_BUILD_TYPE=Debug
```

1. Create `BOOT`, `CONFIGSRV` and `LOG` partition with `prepare_sd_card.sh` script:

```bash
sudo ./prepare_sd_card.sh /dev/<mount-point>
```

2. Copy the RPi3 boot files and the application image `os_image.elf` to the `BOOT`
partition of the SD card.

3. Generate the configuration provisioned binary files (`BLOB.bin`, `DOMAIN.bin`,
`PARAM.bin` and `STRING.bin`):

```bash
seos_tests/seos_sandbox/tools/cpt/build_cpt/cpt -i seos_tests/src/demos/demo_iot_app_rpi3/configuration/config.xml
```

For more information on the `cpt` tool, refer to the handbook.

4. Copy these files to the `CONFIGSRV` partition of the SD card.

5. Start the mosquitto server application:

```bash
docker run -it -p 8883:8883 -v /home/lukas/Desktop/seos_tests/src/demos/demo_iot_app_rpi3/mosquitto_configuration:/mosquitto/config/ eclipse-mosquitto
```

Look into the handbook for more information on how to set up the mosquitto
server.

6. Configure the ethernet interface of the laptop to ip address `10.0.0.1`.

7. Connect RPi3 with ethernet cable to laptop.

8. Power on the RPi3.

9. Interrupt booting and jump into U-Boot command line.

10. Enter the following information in the U-Boot command line:

```bash
u-boot> setenv serverip 10.0.0.1
u-boot> setenv ipaddr 10.0.0.11
u-boot> setenv netmask 255.255.255.0
u-boot> saveenv
u-boot> boot
```
