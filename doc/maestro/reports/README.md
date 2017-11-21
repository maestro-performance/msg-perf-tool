Maestro: performance reports
============

Introduction:
----

This document details the file naming, format and standards used to store the 
performance test data. It is intended for users willing to create their own 
Maestro-compatible performance tools.

As a result of the test execution, the code generates performance reports that 
can be analyzed by other tools. The name and format of the files vary according
to the peer role.


File Names 
----

**Receiver**: 

The receiver peers should generate 2 files: ```receiverd-latency.hdr``` and 
```receiverd-rate.csv.gz```. The first file is used to store the receiver 
latency data and the second is used to store the rate (throughput) data. 

**Receiver Latency**: 

The first file contains latency data in compressed 
HDR Histogram log format (details about the format are available 
[here](http://hdrhistogram.github.io/HdrHistogram/JavaDoc/org/HdrHistogram/HistogramLogWriter.html) 
and [here](http://hdrhistogram.github.io/HdrHistogram/JavaDoc/org/HdrHistogram/HistogramLogReader.html) 
and [here](http://hdrhistogram.github.io/HdrHistogram/JavaDoc/)).

A compliant file would look like this: 

```
#[mpt]
#[Histogram log format version 1.2]
#[StartTime: 1508411972.681 (seconds since epoch), Thu Oct 11:19:32 GMT 2017]
"StartTimestamp","EndTimestamp","Interval_Max","Interval_Compressed_Histogram"
#[mpt]
#[Histogram log format version 1.2]
#[StartTime: 1508411972.681 (seconds since epoch), Thu Oct 11:19:32 GMT 2017]
"StartTimestamp","EndTimestamp","Interval_Max","Interval_Compressed_Histogram"
9226.157,9236.662,103.0,HISTFAAAAJt4nJNpmSzMwMBQyQABzFCaEYhFZK8nMNh/gAi0OF1qYPyQ9SlzVfqxxEsZx1K2xSxJuhHzK35R3K3ohrhlYQ+SLiSciv+QeiD5Ue6C5LbSH+VtBZNiT/lscXyjeU34B8cn5hmMTjYqTmYGbKosAkxCbGIsQiw8XBx8LEIiTAIcQjxABqMYD4cQi4AQBxeQFmAUYxTi4wEAuYstOA==
```

**Receiver Rate**:
 
The receiver rate - the amount of data received by the receiving peer - is 
stored in a compressed comma-separated values (CSV) file. The file must have 
two columns: 

* eta: represents the estimated time of arrival (ETA) for the message, relative to the start of the test
* ata: represents the actual time of arrival (ETA) for the message, relative to the start of the test 

Note: the contents of the column are stored with microsecond precision. 

```
eta;ata
"2017-10-19 13:19:33.9287","2017-10-19 13:19:33.36751"
"2017-10-19 13:19:33.9487","2017-10-19 13:19:33.36796"
"2017-10-19 13:19:33.9687","2017-10-19 13:19:33.36825"
```


The peers are free to sample the rate at any desired interval. The front-ends 
processing the data need to be able to identify and adjust their reports 
accordingly. The front ends must use the actual time of arrival to compute the 
rate. 

**Sender**:

The sending peers should generate 1 file: ```senderd-rate.csv.gz```. This file
should store the sender rate (throughput) data. 

**Sender Rate**:

The sender rate - the amount of data received by the receiving peer - is 
stored in a compressed comma-separated values (CSV) file. The file must have 
two columns: 

* etd: represents the estimated time of departure (ETD) for the message, relative to the start of the test. It may also 
be referenced as ```eta```. 
* atd: represents the actual time of departure (ETD) for the message, relative to the start of the test. It may also 
be referenced as ```ata```. 

Note: the contents of the column are stored with microsecond precision. 

```
etd;atd
"2017-10-19 13:19:32.661300","2017-10-19 13:19:32.706649"
"2017-10-19 13:19:32.661500","2017-10-19 13:19:32.706823"
"2017-10-19 13:19:32.661700","2017-10-19 13:19:32.706917"
```

The peers are free to sample the rate at any desired interval. The front-ends 
processing the data need to be able to identify and adjust their reports 
accordingly. The front ends must use the actual time of departure to compute the 
rate. 


**Inspector**:

The broker inspector peers should generate 1 file: ```brokerd-rate.csv.gz```. This file
should store broker-specific telemetry data. 

The broker inspector file is composed of the following columns:

* timestamp: the date and time for the data sample in the format `%Y-%m-%d %H:%M:%S` (according to Glibc's strftime specification) or `YYYY-MM DD hh:mm:ss` using the W3C defined standard for [datetime](https://www.w3.org/TR/NOTE-datetime).
* load: system load
* open fds: number of opened filed descriptors.
* free fds: number of free file descriptors.
* free mem: free physical memory.
* swap free: swap free memory.
* swap committed: swap committed memory.
* eden inital: eden initial memory.
* eden committed: eden committed memory.
* eden max: eden maximum (limit) memory.
* eden used: eden used memory.
* survivor inital: survivor initial memory.
* survivor committed: survivor committed memory
* survivor max: survivor maximum (limit) memory.
* survivor used: survivor used memory.
* tenured inital: tenured inital memory.
* tenured committed: tenured committed memory.
* tenured max: tenured max memory.
* tenured used: tenured used memory.
* pm inital: permgerm or metaspace initial memory (either permgen or metaspace depending the JVM version).
* pm committed: permgerm or metaspace committed memory (either permgen or metaspace depending the JVM version).
* pm max: permgerm or metaspace maximum memory (either permgen or metaspace depending the JVM version).
* pm used: permgerm or metaspace used memory (either permgen or metaspace depending the JVM version).
* queue size: number of messages waiting for processing on the queue.
* consumers: number of consumers connected to the queue.
* ack: number of acknowledged messages on the queue.
* exp: number of expired messages on the queue.

Sample file:

```
"timestamp","load","open fds","free fds","free mem","swap free","swap committed","eden initial","eden committed","eden max","eden used","survivor initial","survivor committed","survivor max","survivor used","tenured initial","tenured committed","tenured max","tenured used","pm initial","pm committed","pm max","pm used","queue size","consumers","ack","exp"
"2017-10-05 15:06:31",0.0,634,3462,11828,11828,14427,256,340,1364,13,42,0,0,0,683,1162,2731,927,0,43,17592186044415,42,44,4,250,0
"2017-10-05 15:06:41",0.0,634,3462,11829,11829,14427,256,340,1364,167,42,0,0,0,683,1162,2731,927,0,43,17592186044415,42,51,4,32747,0
"2017-10-05 15:06:51",0.0,634,3462,11828,11828,14427,256,340,1364,319,42,0,0,0,683,1162,2731,927,0,43,17592186044415,42,22,4,65158,0
```

Test Properties
----

Every peer must store the test properties/settings used by the test execution. 
These file are used by front-ends to add environment, SUT and test details to
the generated reports.  

**Receiver**:

Stores the test properties in a file name ```test.properties``` located in the test
log directory (the same directory that contains the reports). 

Sample:

```
brokerUri=amqp://hostname/test.performance.queue
durationType=time
duration=300
parallelCount=2
messageSize=25
variableSize=1
rate=9000
fcl=1500
apiName=Qpid Proton C - Messenger API
apiVersion=0.17.0
```

**Sender**:

Stores the test properties in a file name ```test.properties``` located in the test
log directory (the same directory that contains the reports).

```
brokerUri=amqp://hostname/test.performance.queue
durationType=time
duration=300
parallelCount=2
messageSize=25
variableSize=1
rate=9000
fcl=1500
apiName=Qpid Proton C - Messenger API
apiVersion=0.17.0
```

**Inspector**:

Stores the test properties in a file name ```test.properties``` located in the test
log directory (the same directory that contains the reports). Also store the broker 
(SUT) details in a file named ```broker.properties```. The ```test.properties``` file
must have the same format as the one used by the sender and receiver peers.

Sample ```broker.properties```.

```
jvmName=OpenJDK 64-Bit Server VM
jvmVersion=1.8
jvmPackageVersion=1.8.0_141
operatingSystemName=Linux
operatingSystemArch=amd64
operatingSystemVersion=3.10.0-693.el7.x86_64
systemCpuCount=16
systemMemory=16610594816
systemSwap=8455712768
productName=Artemis
productVersion=2.0.0.amq-70001
```

File Location
----

The files may be located on any directory in the file system and accessible via 
the Maestro Data Server.


Log Directory Layout
----

The log files must be stored separately for each test execution. The directory must be named
with the immediately subsequent value for the test execution number, starting from 0 (ie.: the logs
for the first test execution will be stored in ```/var/log/mpt/0```, the second ```/var/log/mpt/1```,
etc). Control process, daemons data and other logs not directly related to the test can be stored in 
the parent directory for the log (ie.: ```/var/log/mpt```).

When the test is completed, the peer must create the following symlinks: 

* ```last```: this link must point to the last test log directory.
* ```lastSuccessful```: this link must point to the last successful test log directory.
* ```lastFailed```: this link must point to the last failed test log directory.

Example (reduced for clarity): 

```
drwxr-xr--. 2 root root   86 Oct  5 15:06 88
drwxr-xr--. 2 root root   86 Sep 28 00:13 9
lrwxrwxrwx. 1 root root   15 Oct  5 16:00 last -> /var/log/mpt/88
lrwxrwxrwx. 1 root root   15 Oct  4 12:44 lastFailed -> /var/log/mpt/73
lrwxrwxrwx. 1 root root   15 Oct  5 16:00 lastSuccessful -> /var/log/mpt/88
-rw-r--r--. 1 root root 1513 Oct  3 00:28 mpt-broker-inspector-11250.log
```

Data Server
----

The data server can be any HTTP server running on port 8000 of the same host as
the peer for which it is serving data.
