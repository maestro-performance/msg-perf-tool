MPT: messaging performance tool
============

Build Status
----
Linux Build Status: [![Linux Build Status](https://travis-ci.org/orpiske/msg-perf-tool.svg?branch=master)](https://travis-ci.org/orpiske/msg-perf-tool) 

Introduction:
----

MPT is a tool for running performance tests on messaging systems current development
version supports AMQP and STOMP messaging protocols. Support for MQTT and OpenWire 
is planned for the future. The test data is saved in a CSV format and can be exported
to ElasticSearch DB. That allows it to be visualized using the 
[Messaging Performance UI](https://github.com/orpiske/msg-perf-ui)

Dependencies:
----

Runtime/Compilation:
* cmake
* gcc or clang
* qpid-proton-c-devel
* [litestomp](https://github.com/orpiske/litestomp) (optional) for STOMP support
* python

Recommended:
* iperf (as a good practice, for testing network performance prior to test execution)


Requirements
----
Disk Space:
The clients may generate a lot of data depending on how much messages are sent
per second. On my baseline system (two servers with Quad-Core AMD Opteron 2376 @ 8x 2.3GHz)
on a gigabit network, it generates around 1Gb of data per hour, transferring
around 66.000 messages per second.

Operating Systems:
* Linux: x86 and x86_64
* OS X: x86


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


Usage - Performance Tool:
----

Here's an example of how to run a 10 minute load test, with 4 concurrent senders,
4 concurrent receivers, sending 256 bytes of data per message. After test, the
performance report is uploaded via SCP to the server pointed by <user>@<host>:<path>

```
mpt-runner.sh -l /tmp/log -b amqp://<amqp server>:5672/<queue name> -d 10 -p 4 -s 256 -u <user>@<host>:<path> -n "sample test"
```

It's possible to have more complex deployment scenarios by running the sender and receiver separately. In this case, you have to
run the test steps manually:

Run the receiver (the controller will print the PID, please take note of that):
```
mpt-receiver -b amqp://<amqp server>:5672/<queue name> --log-level=stat -d 10 -p 4 --logdir=/tmp/log --daemon
```

Run the sender (the controller will print the PID, please take note of that):

```
mpt-sender -b amqp://<amqp server>:5672/<queue name> --log-level=stat -d 10 -p 4 --logdir=/tmp/log --daemon
```

Usage - Runner:
----

Dealing with the synchronization and parameters of performance can be daunting, 
though, therefore a runner script is available to simplify the execution. Before
running the runner, it's advised to create configuration files for both the 
application as well as the test scenario. The file configuration should be simple 
and self-explanatory, since they match the same name of the test parameters.


```
mpt-runner.sh -l /path/to/log -t 5000 -b <protocol>://<host>:<port>/queue/<queue> -d 5 -p 1 -s 32 -C /path/to/mpt-loader.conf -T /path/to/stomp-small-test.conf -R 002
```


Binaries
----

Binaries for this tool, for Fedora, CentOS and RHEL, can be found on my COPR at
https://copr.fedorainfracloud.org/coprs/orpiske/msg-perf-tool/


Tips
----

* Run the clients and the broker in different servers
* Make sure that the time is properly set on both servers
* Run on an dedicated network (or, at least, avoid hours of peak usage)
* Measure the network performance before running (ie.: use iperf)
* Ideally, you should run at a fixed rate instead of flooding the brokers.
Brokers tend to get slower as the queue size increase.


License
----

The code is licensed under Apache License v2
