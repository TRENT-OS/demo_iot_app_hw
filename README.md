# demo_iot_app

Demo application that showcases a rather typical "IoT" use case. A Sensor component cyclically sends an MQTT string to a CloudConnector component. As the name already suggests, this component then opens up a TLS-Connection to an MQTT-Server and publishes the MQTT message to it.
The configuration for the NetworkStack component, the Sensor and the CloudConnector can be set in the "system_config.h" file.
When running the demo, the Admin component of the system will pick up the configuration that can be set for this demo and make use of the PartitionManager component to write these config parameters into a file. Once the configuration file has been successfully written, the ConfigServer component starts up and initializes the backend reading out the parameters of this file. The NetworkStack, Sensor and CloudConnector component will all contact the ConfigServer and retrieve required configuration parameters through it.
Once all component have initialized, the Sensor will proceed to contact the CloudConnector every 5 seconds to send its data string to the configured cloud service.
The demo is set to connect to the Microsoft Azure Cloud.


### Build

The project is meant to be built as part of the seos_tests project
(https://bitbucket.hensoldt-cyber.systems/projects/HC/repos/seos_tests)

To build this demo, you can simply run the following command from the docker container but outside the source folder:

    src/build.sh demo_iot_app

Additionally, you have to build the Proxy but from outside the docker container:

    src/proxy/build.sh ../seos_sandbox

### Run

To run this demo, you first need to create the necessary TAP devices for the NetworkStack. As a prerequirement to that, you will need to have a ethernet connection going. The creation of TAP devices currently does not support wireless connections.
Given the mentioned preconditions, call the create_tap script:

    sudo ./create_tap.sh

The QEMU will need to be started with the UART0 mapped to a TCP port. Run the following command directly out of the build directory:

    qemu-system-arm -machine xilinx-zynq-a9  -nographic -s -serial tcp:localhost:4444,server \
    -serial mon:stdio -m size=1024M  -kernel <IMAGE_PATH>

On startup, QEMU will wait for an incoming connection from the proxy application in a given port (e.g. 4444).
Start the proxy from inside the build folder of the proxy with:

    ./proxy_app -c TCP:4444 -p 8883 -t 1

In the first window, you can now enter 'CTRL+A' and then 'c' to start the run.