# demo_iot_app

Demo application that showcases a rather typical "IoT" use case. A Sensor component cyclically sends an MQTT string to a CloudConnector component. As the name already suggests, this component then opens up a TLS-Connection to an MQTT-Server and publishes the MQTT message to it.
The configuration for the NetworkStack component, the Sensor and the CloudConnector can be set in the "system_config.h" file.
When running the demo, the Admin component of the system will pick up the configuration that can be set for this demo and make use of the ConfigServer component to write these config parameters into the config file previously set up by the ConfigServer. Once the configuration file has been successfully written, the Admin component signals to the relevant clients that the configuration is complete. The NetworkStack, Sensor and CloudConnector component will all contact the ConfigServer and retrieve their required configuration parameters through it.
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

To run this demo, just call the "run_demo.sh" script found in this repository and pass it the build directory of the demo and the path to the built proxy application.
Beware however that you will need an ethernet connection on your machine to run this demo. It utilizes TAP devices and cannot be run over a wireless
network connection. The script will start up and check if the required TAP device is already present of if it needs to be created. If it needs to
created, the script will take care of this. However this is an operation that requires elevated privileges, which is sudo is required:

    sudo ./run_demo.sh <path-to-project-build> <path-to-proxy>


Once the setup is done, you can continue QEMU with 'CTRL+A' and then 'c'.

After running the demo you can get rid of the created TAP devices by calling:

    ./delete_tap.sh