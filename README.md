MPT: messaging performance tool
============


Dependencies:
----
cmake qpid-proton-c-devel gcc gcc-c++ 


Recommended:
valgrind perf

Broker Settings: ActiveMQ
----


ActiveMQ may need to have the inactivity monitor disabled. It can be done by 
adding the following setting in the conf/activemq.xml, in the transport connector
setting:

```
transport.useInactivityMonitor=false
```

For example:

```
<transportConnector name="amqp" uri="amqp://0.0.0.0:5672?maximumConnections=1000&amp;wireFormat.maxFrameSize=104857600&amp;transport.useInactivityMonitor=false"/>
```