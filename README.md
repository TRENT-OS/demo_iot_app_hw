# Demo IoT App

Demo application that showcases a rather typical IoT use case. A sensor
component cyclically publishes a temperature update via MQTT to a
cloudConnector component. This component then opens up a TLS-Connection to an
IoT Hub and in turn publishes the MQTT message to it. The configuration for the
nwStack (network stack), the sensor and the cloudConnector component can be set
in the "seos_system_config.h" file. When running the demo, the admin component
of the system will pick up the configuration and make use of the configServer
component to write these parameters into the empty configuration file, which was
previously established by the configServer. Once the configuration file has been
successfully written, the admin component signals the relevant clients that the
configuration is complete. The nwStack, sensor and cloudConnector component will
all contact the configServer and retrieve their required configuration
parameters through it. Once all component have initialized, the sensor will
proceed to contact the cloudConnector every 5 seconds to send its data string to
the configured cloud service. The demo is set to connect to the Microsoft Azure
Cloud.

## Run

To run this demo, just call the "run_demo.sh" script found in this repository
and pass it the build directory of the demo and the path to the built proxy
application.
Beware however that you will need an ethernet connection on your machine to run
this demo. It utilizes TAP devices and cannot be run over a wireless
network connection. The script will start up and check if the required TAP
device is already present of if it needs to be created. If it needs to
created, the script will take care of this. However this is an operation that
requires elevated privileges, which is why sudo is required:

    sudo ./run_demo.sh <path-to-project-build> <path-to-proxy>


Once the setup is done, you can continue QEMU with 'CTRL+A' and then 'c'.

After running the demo you can get rid of the created TAP devices by calling:

    ./delete_tap.sh