MPT: messaging performance tool
============

Build Status
----

Linux Build Status: [![Linux Build Status](https://travis-ci.org/orpiske/msg-perf-tool.svg?branch=master)](https://travis-ci.org/orpiske/msg-perf-tool)


Packaging Status
----

Fedora/RHEL/CentOS (GRU Testing): [![Fedora COPR (Testing Repo) Status](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-testing/package/msg-perf-tool/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-testing/package/msg-perf-tool/)

Fedora/RHEL/CentOS (GRU Stable): [![Fedora COPR (Stable Repo) Status](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-stable/package/msg-perf-tool/status_image/last_build.png)](https://copr.fedorainfracloud.org/coprs/orpiske/orp-tools-stable/package/msg-perf-tool/)


Introduction:
----

MPT is a tool for running performance tests on messaging systems. Current development
version supports AMQP, STOMP and MQTT messaging protocols. Support for OpenWire and others
is planned for the future. The test data is saved in a compressed CSV format.

Installation:
----

The code can be installed via Fedora COPR. Packages are available for CentOS 6, 7, Fedora 25 
or greater and RHEL 6 and 7. For CentOS 6 and RHEL 6, please use the legacy repos (check the notes below). 

**Testing**

1. Enable my testing COPR.

```dnf copr enable orpiske/orp-tools-testing```

2. Install the runtime only:
```dnf install -y msg-perf-tool```

**Note**: the testing packages are the **recommended** packages for this project.

**Stable**

1. Enable my testing COPR.

```dnf copr enable orpiske/orpiske/orp-tools-stable ```

2. Install the runtime only:
```dnf install -y msg-perf-tool```

Dependencies:
----

For building the project, the following dependencies are required:  

* cmake
* gcc >= 4.8.0 or clang
* qpid-proton-c-devel
* [litestomp](https://github.com/orpiske/litestomp) (optional/experimental) for STOMP support
* [paho-c](https://www.eclipse.org/paho/) (optional/experimental) for MQTT support
* [gru](https://github.com/orpiske/gru)
* [bmic](https://github.com/orpiske/bmic)
* python
* zlib
* libuuid
* readline
* msgpack-c
* hdr-histogram-c
* uriparser

To compile the project:
```
mkdir build && build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DSTOMP_SUPPORT=ON -DAMQP_SUPPORT=ON -DMQTT_SUPPORT=ON -DCMAKE_BUILD_TYPE=RELEASE -DCMAKE_USER_C_FLAGS="-fPIC" ..
```

Requirements
----
Disk Space:
The clients may generate a lot of data depending on how much messages are sent
per second. On my baseline system (two servers with Quad-Core AMD Opteron 2376 @ 8x 2.3GHz)
on a gigabit network, it generates around 1Gb of data per hour, transferring
several thousand messages per second (the exact number may vary according to message size,
network bandwidth and broker setup).

Operating Systems:
* Linux: x86 (experimental)and x86_64 (recommended)
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


Usage - Performance Tool CLI
----

Here's an example of how to run a 10 minute and 30 seconds load test sending 256 bytes of data 
per message. Each command has to be run on a separate shell:

```
mpt-sender -d 10m30s -l debug -s 256 -b amqp://<hostname>/<queue name>
```

```
mpt-receiver -d 10m30s -l debug -s 256 -b amqp://<hostname>/<queue name>
```

**Note**: the CLI tool should be used for simple testing, smoke testing and other small scale verifications. It 
does not provide any mechanism for test orchestration or management. 


Usage - Performance Tool Maestro
----

Maestro is a distributed test orchestration API that can be used to orchestrate the test orchestration between multiple
test nodes. The tests are run by the maestro daemons and the test orchestration can be done either via the maestro CLI or
via a [Java and Groovy-based](https://github.com/orpiske/maestro-java) test orchestration api.

**Maestro Broker**

The maestro broker can be any MQTT-capable message broker. The daemons subscribe to different topics and, thus, receive 
the test instructions via the maestro API.

**Overview**

![Maestro Overview](doc/maestro-overview.png)

**Daemons Configuration**

The daemons configuration is straight forward. Just edit the appropriate daemon file:

* Sender daemon: /etc/sysconfig/mpt-sender-daemon
* Receiver daemon: /etc/sysconfig/mpt-receiver-daemon
* Inspector daemon: /etc/sysconfig/mpt-broker-inspector
* Data server daemon: /etc/sysconfig/mpt-data-server

The configurations to be adjusted are the URL of the maestro broker and the unique NODE_NAME that identifies the node 
on the test cluster. The NODE_NAME **must** be in the format: name@hostname.  


```
MAESTRO_BROKER_URL=mqtt://localhost:1883
NODE_NAME=mynode@hostname.com
```

Additionally, other settings might be adjusted, although the defaults should be just fine. After configured, the daemons
can be started using the the following command line: 

```
systemctl start mpt-sender-daemon
```

**Note**: replace mpt-sender-daemon with mpt-receiver-daemon, mpt-broker-inspector or
mpt-data-server.


**Maestro CLI**

The maestro CLI is a very simple command line interface that allows sending maestro commands to the maestro broker.
To run the CLI, execute:

```
mpt-maestro -m mqtt://localhost:1883
```

On the CLI, several commands can be sent:

```
quit
start-receiver
stop-receiver
start-sender
stop-sender
start-inspector
stop-inspector
start-all
stop-all
collect
flush
set
ping
stats
halt
```

**Note**: the purpose of the CLI is to serve as a way to run ad-hoc commands on the test cluster. For test 
orchestration, the Java or the Groovy API must be used. 


Usage - Runtime Parameters and Message Customization 
----

It is possible to customize protocol-specific parameters so that the tests closely match real world conditions. 
These parameters are usually configured by adjusting the broker URL. For example: ```amqp://host/queue?opt=value```.

The following parameters can be set for AMQP:

| Parameter Name    | Default Value       | Description          |
|-------------------|---------------------|----------------------|
| `content-type` | `text/plain` | Message content type |
| `application-properties` | null | Whether to skip instance creation process |
| `ttl` | 5000 | Time to live. |
| `durable` | true | Durable flag for the message |
| `priority` | null | Message priority (use 'variable' for random)  |
| `qos-mode` | `at-most-once`, `at-least-once`, `exactly-once` | Default QOS mode |


The following parameters can be set for MQTT:
 
| Parameter Name    | Default Value       | Description          |
|-------------------|---------------------|----------------------|
| `keep-alive` | `text/plain` | Keep-alive interval |
| `clean-session` | null | MQTT clean session flag |
| `retained` | null | MQTT retained flag |
| `qos-mode` | `at-most-once`, `at-least-once`, `exactly-once` | Default QOS mode |

 
 **Note**: STOMP does not yet support parameters.
 
 **Note**: always check the protocol specification for the exact meaning of the semantics of the flags.
 
 
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
