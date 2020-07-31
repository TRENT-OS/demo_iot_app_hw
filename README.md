# Demo IoT App

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