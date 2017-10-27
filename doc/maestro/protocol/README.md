Maestro: test orchestration API
============

Introduction:
----

Maestro is the test orchestration API used by the msg-perf-tool in order to set test 
parameters, orchestrate test execution and share testing status among the members of 
the testing cluster.

Maestro Testing Cluster:
----

The testing cluster refers to the loosely connected group of machines that coordinate, 
execute and inspect the test execution and the software under test (SUT). 

The cluster is composed of: 

* A controller node, whose job is to set up test parameters and coordinate the test execution
* One or more testing nodes: whose job is to run the tests and share the testing data.

There are 3 types of testing nodes, based on their responsibility:

* Sender node: whose job is to send data to the SUT instance(s)
* Receiver node: whose job is to receive data from the SUT instance(s)
* Inspector node: that is responsible for inspecting the SUT (when available)

Normally, there are 2 jobs per node: the test controller job, which commands the test execution
(send, receive or inspect),  and a data server that serves the test data after the test has been
executed. 


Maestro Protocol:
----

**Basic Details**

The Maestro protocol is a binary protocol implemented on top of [msgpack](https://msgpack.org/). 
The [MQTT](http://docs.oasis-open.org/mqtt/mqtt/v3.1.1/csprd02/mqtt-v3.1.1-csprd02.html) messaging 
protocol, version 3.1.1, is used to exchange data between the testing nodes. The 
[HTTP](https://www.w3.org/Protocols/HTTP/1.1/rfc2616bis/draft-lafon-rfc2616bis-03.html) protocol, 
version 1.1, is used by the data-server to serve the testing data. 

The messages exchanged between the peers of the testing cluster, via MQTT, are called notes. 

**Format**

The format of the nodes is: 


| Type | Command | Payload |
|------|---------|---------|


**Type**

Type is a short integer that identifies the purpose of the note:

* Request (0): a note sent by a controller node to the test peers.
* Response (1): a note sent by a testing peer as a response for a request. 
* Notification (2): a note sent by a testing peer as a reaction to an event.  

Obs.: the values in parenthesis represent the numeric value of the type. 

**Command**

Identifies the action to be executed or, in some cases, that was executed. There are, 
currently, 17 commands. The command is represented by a long integer. 

Some commands are only applicable to some note types. Also, some commands are only 
applicable to some node types. The commands available are: 

* MAESTRO_NOTE_START_RECEIVER (0)
* MAESTRO_NOTE_STOP_RECEIVER (1)
* MAESTRO_NOTE_START_SENDER (2)
* MAESTRO_NOTE_STOP_SENDER (3)
* MAESTRO_NOTE_START_INSPECTOR (4)
* MAESTRO_NOTE_STOP_INSPECTOR (5)
* MAESTRO_NOTE_FLUSH (6)
* MAESTRO_NOTE_SET (7)
* MAESTRO_NOTE_STATS (8)
* MAESTRO_NOTE_HALT (9)
* MAESTRO_NOTE_PING (10)
* MAESTRO_NOTE_OK (11)
* MAESTRO_NOTE_PROTOCOL_ERROR (12)
* MAESTRO_NOTE_INTERNAL_ERROR (13)
* MAESTRO_NOTE_ABNORMAL_DISCONNECT (14)
* MAESTRO_NOTE_NOTIFY_FAIL (15)
* MAESTRO_NOTE_NOTIFY_SUCCESS (16) 


Obs.: the values in parenthesis represent the numeric value of the command.

**Payload**

The data carried by the note as part of its command.


**Request: MAESTRO_NOTE_START_RECEIVER** 

This note is issued by a controller to the receiver node to request it to start receiving data. 

* Value: 0
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed,


**Request: MAESTRO_NOTE_STOP_RECEIVER**

This note is issued by a controller to the receiver node to request it to stop receiving data. 

* Value: 1
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

**Request: MAESTRO_NOTE_START_SENDER**

This note is issued by a controller to the sender node to request it to start sending data. 

* Value: 2
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

**Request: MAESTRO_NOTE_STOP_SENDER**

This note is issued by a controller to the sender node to request it to stop sending data.

* Value: 2
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.


**Request: MAESTRO_NOTE_START_INSPECTOR**

This note is issued by a controller to the inspector node to request it to start inspecting 
the SUT.

* Value: 4
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.


**Request: MAESTRO_NOTE_STOP_INSPECTOR**

This note is issued by a controller to the inspector node to request it to inspecting the SUT.

* Value: 5
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.


**Request: MAESTRO_NOTE_FLUSH**

This note is issued by a controller to the any node to request it to flush test data to disk.

* Value: 6
* Payload: none
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

**Request: MAESTRO_NOTE_SET**

This note is issued by a controller to the any node to set the testing properties.

* Value: 7
* Payload: yes (described below)
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

**Request: MAESTRO_NOTE_SET Payload**

| Option | Value |
|--------|-------|

* Option: a long that represents the test option being set. 
* Value: a string containing the value to be set.
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` if the request was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

The available options, as well as their acceptable values are:


* MAESTRO_NOTE_OPT_SET_BROKER (0): (aka MAESTRO_NOTE_OPT_SET_ENDPOINT) used to set a URL that the 
peers must send that to or receive data from.
    * Value: a string in the format ```protocol://<hostname>/<queue name>```.
* MAESTRO_NOTE_OPT_SET_DURATION_TYPE (1): sets the duration type and duration of the test. 
    * Value: an integer that represents the number of messages to exchange or properly formatted 
        string representing days, hours, minutes and/or seconds.
        * Example durations: `1d` for 1 day;  `1d1h` for 1 day and 1 hour; `1d1h1m1s` for 1 day, 
        1 hour, 1 minute and 1 second.
* MAESTRO_NOTE_OPT_SET_LOG_LEVEL (2): used to set the log level on the nodes
    * Value: one of trace, debug, info, warning, error or fatal.
* MAESTRO_NOTE_OPT_SET_PARALLEL_COUNT (3): sets the number of concurrent connection for the node.
    * Value: an integer up to 65535.
* MAESTRO_NOTE_OPT_SET_MESSAGE_SIZE (4):
    * Value: a numeric string that represents a fixed message size or a number prefixed with `~` 
    for a variable message size (5% size variation). 
        * Example: `256` for 256 bytes or `~256` for 256 bytes with a 5% size variation.  
* MAESTRO_NOTE_OPT_SET_THROTTLE (5): sets throttling on on the sender. Do not use. Deprecated feature. 
    * Value: a numeric value representing the number of messages to be sent per connection.
* MAESTRO_NOTE_OPT_SET_RATE (6): sets the desired target rate for each connection. 
    * Value: a number that represents the desired number of messages that each connection should try 
    to send.  
* MAESTRO_NOTE_OPT_FCL (7): sets "fail condition on latency". If the latency ever exceeds this value, 
the test fails.
    * Value: the maximum acceptable latency.


**Request: MAESTRO_NOTE_STATS**

This note is issued by a controller to the any node to request the current performance statistics.

* Value: 8
* Payload: no.
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_STATS``` reply if the requested was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

**Request: MAESTRO_NOTE_HALT**


This note is issued by a controller to the any node to request them to stop and exit cleanly.

* Value: 9
* Payload: no.
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_OK``` reply if the requested was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

**Request: MAESTRO_NOTE_PING**


This note is issued by a controller to the any node to verify which peers are alive in the cluster.

* Value: 10
* Payload: yes.
* Response: the peers respond to this note by sending a ```MAESTRO_NOTE_PING``` reply if the requested was
successful or ```MAESTRO_NOTE_INTERNAL_ERROR``` if the request failed.

| Seconds | Microsends |
|---------|------------|

* Seconds: seconds since epoch in the controller node.
* Microseconds: microseconds within the second.

**Responses:**

| Id | Name | Body |
|----|------|------|

* ID: the ID, represented as a UUID, of the node. 
* Name: the name of the node in the format ```test_node_type@FQDN```
* Body: the response body


**Response: MAESTRO_NOTE_PROTOCOL_ERROR**

This note is issued by any node whenever the protocol is malformed.

* Value: 12
* Payload: no.

**Response: MAESTRO_NOTE_INTERNAL_ERROR**

This note is issued by any node when it is unable to comply with a request.

* Value: 13
* Payload: no.

**Response: MAESTRO_NOTE_ABNORMAL_DISCONNECT**

This note is issued by any node as a last-will message.

* Value: 14
* Payload: no.

Note: this note is incorrectly mapped as response but will be changed to notification in the future.


**Response: MAESTRO_NOTE_OK**

This note is a generic response when the node complies with a request

* Value: 11
* Payload: no.

**Response: MAESTRO_NOTE_STATS**

This note is sent by a node as a response to a ```MAESTRO_NOTE_STATS``` request.

* Value: 8
* Payload: yes (see below).


| Child Count | Role | Role Info | Stats Type | Perf Data |
|-------------|------|-----------|------------|-----------|

* Child Count: number of connections
* Role (sender, receiver, inspector)
* Stats type (unused)
* Perf data (structure, see below)


Perf Data Format: 

| Timestamp | Count | Rate | Latency |
|-----------|-------|------|---------|


* Timestamp: epoch seconds.microseconds
* Count: number of messages exchanged 
* Rate: current rate
* Latency: latency (when applicable)

Note: unstable interface which may change in the future.

**Response: MAESTRO_NOTE_PING**

This note is sent by the peers as a response to a ```MAESTRO_NOTE_PING``` request.

* Value: 10
* Payload: yes (see below)

| Elapsed |
|---------|

* Elapsed: the elapsed time, in milliseconds, between the request and receiving it on the peer. 


**Notify: MAESTRO_NOTE_NOTIFY_FAIL**

This note is issued by any node when the test failed.

* Value: 15
* Payload: yes.

| Message |
|---------|

* Message: the error message for the failure


**Notify: MAESTRO_NOTE_NOTIFY_SUCCESS**
 
This note is issued by any node when the test completed successfully.
 
 * Value: 16
 * Payload: yes.
 
 | Message |
 |---------|
 
 * Message: the success message (if any).

