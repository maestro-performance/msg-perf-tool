#!/usr/bin/env python

"""
 Copyright 2016 Otavio Rodolfo Piske

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

 http://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
"""

__author__ = 'opiske'

import ConfigParser
import csv
import json
import logging
import optparse
import sys
from StringIO import StringIO

import requests
from requests.auth import HTTPBasicAuth

fmt_console = '[%(asctime)s] [%(levelname)s] %(name)s :: %(message)s'
datefmt_iso = '%Y-%m-%d %H:%M:%S,'

console_formatter = logging.Formatter(fmt_console, datefmt=datefmt_iso)

console_handler = logging.StreamHandler()  # Python 2.6
console_handler.setLevel(logging.ERROR)  # setting console level
console_handler.setFormatter(console_formatter)

logging.getLogger().addHandler(console_handler)
logging.getLogger().setLevel(logging.NOTSET)

logger = logging.getLogger(__name__)
config = ConfigParser.RawConfigParser();
in_opts = {};


def print_errors(http_response):
    logger.error("Error: %s" % http_response.status_code)
    logger.error("Text: %s" % http_response.content)

def read_param(section, name, default=None):
    ret = in_opts[name]
    if ret is None:
        try:
            ret = config.get(section, name)
            if ret is None and default is not None:
                ret = default
        except:
            ret = None

    return ret;


def call_service(req_url, request_json, force_update=False, session=None, is_update=False):
    headers = {'Content-type': 'application/json'}

    username = read_param("database", "username")
    if username is not None:
        password = read_param("database", "password")

    if session is None:
        session = requests.session()

    logger.debug("Req URL: %s" % req_url)
    logger.debug("Data: %s" % request_json)

    if is_update and not force_update:
        logger.info("Executing HTTP PUT to %s" % (req_url))
        answer = session.put(req_url, headers=headers, data=request_json, verify=False,
                               auth=HTTPBasicAuth(username, password))
    else:
        logger.info("Executing HTTP POST to %s" % (req_url))
        answer = session.post(req_url, headers=headers, data = request_json, verify=False,
                               auth=HTTPBasicAuth(username, password))

    if answer.status_code < 200 or answer.status_code >= 205:
        print_errors(answer)
        return answer.status_code

    return 0


def call_service_for_check(req_url, session=None):
    logger.debug("Connecting to %s" % (req_url))

    headers = {'Content-type': 'application/json'}

    username = read_param("database", "username")
    if username is not None:
        password = read_param("database", "password")

    if session is None:
        session = requests.session()

    answer = session.get(req_url, headers=headers, verify=False, auth=HTTPBasicAuth(username, password))
    return answer

def get_index_name():
    in_test_run = read_param("test", "test_run")
    in_start_time = in_opts["test_start_time"]
    in_sut_key = read_param("sut", "sut_key")
    in_sut_version = read_param("sut", "sut_version")

    index_time = in_start_time.split()[0]

    vkey=in_sut_version.replace(".", "").replace("-", "")
    return "%s-%s-%s-%s" % (in_sut_key, vkey, in_test_run, index_time)

def register():
    base_url = read_param("database", "url")
    in_sut_name = read_param("sut", "sut_name")
    in_sut_key = read_param("sut", "sut_key")
    in_sut_version = read_param("sut", "sut_version")

    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_sut_name is None or in_sut_name == "":
        logger.error("The SUT name is required")
        return 1

    if in_sut_key is None or in_sut_key == "":
        logger.error("The SUT key is required")
        return 1

    if in_sut_version is None or in_sut_version == "":
        logger.error("The SUT version is required")
        return 1

    # TODO: this is not good nor flexible. Fix it!
    filtered_version = in_sut_version.replace(".", "").replace(" ", "").replace("-", "").replace("_", "")
    sut_id = "%s-%s" % (in_sut_key, filtered_version)

    request_data = {
        "sut_name": in_sut_name,
        "sut_key": in_sut_key,
        "sut_version": in_sut_version,
    }

    logger.info("Request: %s" % (request_data))

    request_json = StringIO()
    json.dump(request_data, request_json)
    logger.info("JSON: %s" % (request_json.getvalue(),))

    is_update = in_opts["update"]
    req_url = "%s/sut/messaging/%s" % (base_url, sut_id)
    call_service(req_url, request_json.getvalue(), is_update=is_update)

    return 0

def configure_cache(session=None):
    base_url = read_param("database", "url")
    in_sut_key = read_param("sut", "sut_key")
    in_start_time = in_opts["test_start_time"]

    index_time = in_start_time.split()[0]

    logger.debug("Configuring index cache for test data")

    req_url = "%s/%s-%s" % (base_url, in_sut_key, index_time)
    request_json = '{ "index.cache.query.enable": true }'

    # It will return 400 if already configured
    ret = call_service(req_url, request_json, force_update=True, session=session)
    if ret != 0 and ret != 400 :
        logger.error("Unable to configure the index cache for test data")

def configure_latency_mapping(session=None):
    base_url = read_param("database", "url")

    index_name = get_index_name();
    logger.debug("Configuring latency mapping")

    answer = call_service_for_check(("%s/%s/_mapping" % (base_url, index_name)), session=session)
    if answer.status_code == 404:
        req_url = "%s/%s" % (base_url, index_name)
        request_json = '{ "mappings": { "receiver-latency": { "properties": { "creation": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss.SSS"}, "sut_version": { "type": "string", "index": "not_analyzed"  } } } } }'
    else:
        req_url = "%s/%s/_mapping/receiver-latency" % (base_url, index_name)
        request_json = '{ "properties": { "creation": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss.SSS"}, "sut_version": { "type": "string", "index": "not_analyzed" } } }'

    is_update = in_opts["update"]
    ret = call_service(req_url, request_json, force_update=True, session=session, is_update=is_update)
    if ret != 0:
        logger.error("Unable to configure the mappings for latency")

def configure_throughput_mapping(session=None, direction=None):
    base_url = read_param("database", "url")
    index_name = get_index_name()

    answer = call_service_for_check(( "%s/%s/_mapping" % (base_url, index_name)), session=session)
    if answer.status_code == 404:
        req_url = "%s/%s" % (base_url, index_name)
        request_json = '{ "mappings": { "mpt-%s-throughput": { "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"}, "sut_version": { "type": "string", "index": "not_analyzed" } } } } }' % direction
    else:
        req_url = "%s/%s/_mapping/mpt-%s-throughput" % (base_url, index_name, direction)
        request_json = '{ "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"}, "sut_version": { "type": "string", "index": "not_analyzed" } } }'

    is_update = in_opts["update"]
    ret = call_service(req_url, request_json, force_update=True, session=session, is_update=is_update)
    if ret != 0:
        logger.error("Unable to configure the mappings for throughput")

def configure_testinfo_mapping(session=None):
    base_url = read_param("database", "url")
    logger.debug("Configuring test info mapping")

    answer = call_service_for_check(("%s/test/info/_mapping" % (base_url)), session=session)
    if answer.status_code == 404:
        req_url = "%s/test" % (base_url)
        request_json = '{ "mappings": { "info": { "properties": { "sut_version": { "type": "string", "index": "not_analyzed" } } } } }'
    else:
        req_url = "%s/test/_mapping/info" % (base_url)
        request_json = '{ "properties": { "sut_version": { "type": "string", "index": "not_analyzed" } } }'

    is_update = in_opts["update"]
    ret = call_service(req_url, request_json, force_update=True, session=session, is_update=is_update)
    if ret != 0:
        logger.error("Unable to configure the mappings for latency")

def count_lines(datafile):
    i = 0
    for i, l in enumerate(datafile):
      pass

    datafile.seek(0)
    return i + 1

def validate_parameters():
    base_url = read_param("database", "url")
    in_file_name = in_opts["filename"]
    in_sut_name = read_param("sut", "sut_name")
    in_sut_key = read_param("sut", "sut_key")
    in_sut_version = read_param("sut", "sut_version")
    in_test_run = read_param("test", "test_run")
    in_direction = in_opts["msg_direction"]

    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_file_name is None:
        logger.error("Input filename is required")

        return 1

    if in_sut_name is None:
        logger.error("SUT name is required")

        return 1

    if in_sut_key is None:
        logger.error("Key is required")

        return 1

    if in_sut_version is None:
        logger.error("Version is required")

        return 1

    if in_test_run is None:
        logger.error("Test run is required")

        return 1

    if in_direction is None:
        logger.error("Direction is required")

        return 1

    return 0

def load_latencies_bulk():
    base_url = read_param("database", "url")
    in_file_name = in_opts["filename"]
    in_direction = in_opts["msg_direction"]

    param_check = validate_parameters()
    if param_check != 0:
        return param_check

    index_name = get_index_name();

    session = requests.session();

    ret = call_service_for_check("%s/test/info/%s" % (base_url, index_name), session=session)
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % index_name)

        return 1

    datafile = open(in_file_name, 'rb')
    num_lines = (count_lines(datafile) - 1)

    configure_latency_mapping(session=session)

    logger.info("There are %d lines to read" % (num_lines))
    csv_data = csv.reader(datafile, delimiter=';', quotechar='|')
    i = 0;

    req_url = "%s/%s/_bulk" % (base_url, index_name)

    action_data = {
        "index": {
            "_type": ("mpt-%s-latency" % in_direction)
        }
    }

    quiet = in_opts["quiet"]
    bulk_json = StringIO();
    skipped = 0

    for row in csv_data:
        # Skip the headers
        if i == 0 and row[0] == "creation":
            continue

        if len(row) < 2:
            skipped += 1;
            continue

        creation = row[0]
        latency = int(row[1])

        latency_data = {
            "latency": latency,
            "creation": creation,
        }

        json.dump(action_data, bulk_json)
        bulk_json.write('\n')

        json.dump(latency_data, bulk_json)
        bulk_json.write('\n')

        i += 1

        if (i % 1000) == 0:
            if not quiet:
                sys.stdout.write("Bulk uploading latency data (%d records out of %d)\r" % (i, num_lines))

            call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

            bulk_json.truncate(0)
            bulk_json.seek(0)

    call_service(req_url , bulk_json.getvalue(), session=session, is_update=True)

    if not quiet:
        print ""

    if skipped > 0:
        logger.warn("Skipped invalid %d records", skipped)

    datafile.close()
    bulk_json.close()

    configure_cache(session=session)
    return 0

def load_throughput_bulk():
    base_url = read_param("database", "url")
    in_file_name = in_opts["filename"]
    in_msg_direction = in_opts["msg_direction"]

    param_check = validate_parameters()
    if param_check != 0:
        return param_check

    index_name = get_index_name()

    session = requests.session()
    ret = call_service_for_check("%s/test/info/%s" % (base_url, index_name), session=session)
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % index_name)

        return 1

    datafile = open(in_file_name, 'rb')
    num_lines = (count_lines(datafile) - 1)

    configure_throughput_mapping(session=session, direction=in_msg_direction)

    logger.info("There are %d lines to read" % (num_lines))
    csv_data = csv.reader(datafile, delimiter=';', quotechar='|')

    req_url = "%s/%s/_bulk" % (base_url, index_name)

    action_data = {
        "index": {
            "_type": ("mpt-%s-throughput" % in_msg_direction)
        }
    }

    quiet = in_opts["quiet"]
    bulk_json = StringIO();

    i = 0;
    skipped = 0;
    for row in csv_data:
        # Skip the headers
        if i == 0 and row[0] == "timestamp":
            continue

        if len(row) < 4:
            skipped += 1
            continue

        ts = row[0]
        count = int(row[1])
        duration = int(row[2])
        rate = float(row[3])

        throughput_data = {
            "ts": ts,
            "count": count,
            "duration": duration,
            "rate": rate,
        }

        json.dump(action_data, bulk_json)
        bulk_json.write('\n')

        json.dump(throughput_data, bulk_json)
        bulk_json.write('\n')

        i += 1

        if (i % 1000) == 0:
            if not quiet:
                sys.stdout.write("Bulk uploading throughput data (%d records out of %d)\r" % (i, num_lines))
            call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

            bulk_json.truncate(0)
            bulk_json.seek(0)

    call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

    if not quiet:
        print ""

    if skipped > 0:
        logger.warn("Skipped invalid %d records", skipped)

    datafile.close()
    bulk_json.close()

    configure_cache(session=session)
    return 0

def load_test_info():
    """
    Loads test information into the database
    :param in_opts: input options from command line
    :return:
    """
    base_url = read_param("database", "url")
    in_sut_key = read_param("sut", "sut_key")
    in_sut_version = read_param("sut", "sut_version")

    in_test_run = read_param("test", "test_run")
    in_start_time = in_opts["test_start_time"]
    in_test_duration = read_param("test", "test_duration")
    in_test_comment = read_param("test", "test_comment")
    in_test_result_comment = in_opts["test_result_comment"]

    in_broker_os_name = read_param("broker", "broker_os_name")
    in_broker_os_type = read_param("broker", "broker_os_type")
    in_broker_os_version = read_param("broker", "broker_os_version")
    in_broker_hw_type = read_param("broker", "broker_hw_type")
    in_brk_sys_info = read_param("broker", "broker_sys_info")

    in_msg_protocol = read_param("messaging", "msg_protocol")
    in_msg_size = read_param("messaging", "msg_size")
    in_msg_endpoint_type = read_param("messaging", "msg_endpoint_type")

    in_prod_count = read_param("producer", "producer_count")
    in_prod_sys_info = read_param("producer", "producer_sys_info")

    in_con_count = read_param("consumer", "consumer_count")
    in_con_sysinfo = read_param("consumer", "consumer_sys_info")


    if base_url is None:
        logger.error("Base URL is required")

        return 1

    if in_test_run is None:
        logger.error("Test run is required")

        return 1

    if in_start_time is None:
        logger.error("Test start time is required")

        return 1

    if in_sut_key is None:
        logger.error("SUT key is required")

        return 1

    configure_testinfo_mapping()

    index_name = get_index_name();

    ret = call_service_for_check("%s/sut/messaging/_search?q=key:%s" % (base_url, in_sut_key))
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no SUT with the ID %s. Please load that key before recording a test info"
                     % index_name)

    req_url = "%s/test/info/%s" % (base_url, index_name)

    request_data = {
        "test_id": index_name,
        "test_run": in_test_run,
        "test_req_url": index_name,
        "sut_key": in_sut_key,
        "sut_version": in_sut_version,
        "test_start_time": in_start_time,
        "test_duration": in_test_duration,
        "msg_protocol": in_msg_protocol,
        "msg_size": in_msg_size,
        "msg_endpoint_type": in_msg_endpoint_type,
        "broker_os_name": in_broker_os_name,
        "broker_os_type": in_broker_os_type,
        "broker_os_version": in_broker_os_version,
        "broker_hw_type": in_broker_hw_type,
        "broker_sysinfo": in_brk_sys_info,
        "producer_count": in_prod_count,
        "producer_sysinfo": in_prod_sys_info,
        "consumer_count": in_con_count,
        "consumer_sysinfo": in_con_sysinfo,
        "test_comment": in_test_comment,
        "test_result_comment": in_test_result_comment,
    }

    request_json = StringIO()
    json.dump(request_data, request_json)

    logger.info("JSON: %s" % (request_json.getvalue()))

    is_update = in_opts["update"]
    call_service(req_url, request_json.getvalue(), is_update=is_update)

    return 0

def configure_network_mapping(session=None):
    base_url = read_param("database", "url")
    index_name = get_index_name();

    answer = call_service_for_check(("%s/%s/_mapping" % (base_url, index_name)), session=session)
    if answer.status_code == 404:
        req_url = "%s/%s" % (base_url, index_name)
        request_json = '{ "mappings": { "sender-network": { "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"}, "sut_version": { "type": "string", "index": "not_analyzed" } } } } }'
    else:
        req_url = "%s/%s/_mapping/sender-network" % (base_url, index_name)
        request_json = '{ "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"}, "sut_version": { "type": "string", "index": "not_analyzed" } } }'

    is_update = in_opts["update"]
    ret = call_service(req_url, request_json, force_update=True, session=session, is_update=is_update)
    if ret != 0:
        logger.error("Unable to configure the mappings for network")

def load_sender_network_info():
    base_url = read_param("database", "url")
    in_file_name = in_opts["filename"]
    in_msg_direction = in_opts["msg_direction"]

    param_check = validate_parameters()
    if param_check != 0:
        return param_check

    index_name = get_index_name();

    session = requests.session();

    ret = call_service_for_check("%s/test/info/%s" % (base_url, index_name), session=session)
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % index_name)

        return 1

    datafile = open(in_file_name, 'rb')
    num_lines = (count_lines(datafile) - 1)

    configure_network_mapping(session=session)

    logger.info("There are %d lines to read" % (num_lines))
    csv_data = csv.reader(datafile, delimiter=';', quotechar='|')
    i = 0;

    req_url = "%s/%s/_bulk" % (base_url, index_name)

    action_data = {
        "index": {
            "_type": "sender-network"
        }
    }

    quiet = in_opts["quiet"]
    bulk_json = StringIO();
    skipped = 0

    for row in csv_data:
        # Skip the headers
        if i == 0 and row[0] == "timestamp":
            continue

        if len(row) < 2:
            skipped += 1;
            continue

        ts = row[0]
        tx = int(row[1])
        rx = int(row[2])

        latency_data = {
            "ts": ts,
            "rx": rx,
            "tx": tx,
        }

        json.dump(action_data, bulk_json)
        bulk_json.write('\n')

        json.dump(latency_data, bulk_json)
        bulk_json.write('\n')

        i += 1

        if (i % 1000) == 0:
            if not quiet:
                sys.stdout.write("Bulk uploading sender network data (%d records out of %d)\r" % (i, num_lines))

            call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

            bulk_json.truncate(0)
            bulk_json.seek(0)

    call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

    if not quiet:
        print ""

    if skipped > 0:
        logger.warn("Skipped invalid %d records", skipped)

    datafile.close()
    bulk_json.close()

    configure_cache(session=session)
    return 0

def configure_broker_java_mapping(session=None):
    base_url = read_param("database", "url")

    index_name = get_index_name()

    answer = call_service_for_check(("%s/%s/_mapping" % (base_url, index_name)), session=session)
    if answer.status_code == 404:
        req_url = "%s/%s" % (base_url, index_name)
        request_json = '{ "mappings": { "broker-java": { "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"} } } } }'
    else:
        req_url = "%s/%s/_mapping/broker-java" % (base_url, index_name)
        request_json = '{ "properties": { "ts": { "type": "date", "format": "yyyy-MM-dd HH:mm:ss"} } }'

    is_update = in_opts["update"]
    ret = call_service(req_url, request_json, force_update=True, session=session, is_update=is_update)
    if ret != 0:
        logger.error("Unable to configure the mappings for network")

def load_broker_java_info():
    base_url = read_param("database", "url")
    in_file_name = in_opts["filename"]

    param_check = validate_parameters()
    if param_check != 0:
        return param_check

    session = requests.session();

    index_name = get_index_name()

    ret = call_service_for_check("%s/test/info/%s" % (base_url, index_name), session=session)
    if ret.status_code < 200 or ret.status_code >= 205:
        logger.error("There's no test with the ID %s. Please record that test info before loading data"
                     % index_name)

        return 1

    datafile = open(in_file_name, 'rb')
    num_lines = (count_lines(datafile) - 1)

    configure_broker_java_mapping(session=session)

    logger.info("There are %d lines to read" % (num_lines))
    csv_data = csv.reader(datafile, delimiter=';', quotechar='|')
    i = 0;

    req_url = "%s/%s/_bulk" % (base_url, index_name)

    action_data = {
        "index": {
            "_type": "broker-java"
        }
    }

    quiet = in_opts["quiet"]
    bulk_json = StringIO();
    skipped = 0

    for row in csv_data:
        # Skip the headers
        if i == 0 and row[0] == "timestamp":
            continue

        if len(row) < 2:
            skipped += 1;
            continue

        ts = row[0]
        load = row[1]
        open_fds = int(row[2])
        free_fds = int(row[3])
        free_mem = int(row[4])
        swap_free = int(row[5])
        swap_cmm = int(row[6])
        eden_ini = int(row[7])
        eden_cmm = int(row[8])
        eden_max = int(row[9])
        eden_used = int(row[10])
        svv_ini = int(row[11])
        svv_cmm = int(row[12])
        svv_max = int(row[13])
        svv_used = int(row[14])
        tnd_ini = int(row[15])
        tnd_cmm = int(row[16])
        tnd_max = int(row[17])
        tnd_used = int(row[18])
        pm_ini = int(row[19])
        pm_cmm = int(row[20])
        pm_max = int(row[21])
        pm_used = int(row[22])
        queue_size = int(row[23])
        consumers = int(row[24])
        msg_ack = int(row[25])
        msg_exp = int(row[26])

        sys_data = {
            "ts": ts,
            "load": load,
            "open_fds": open_fds,
            "free_fds": free_fds,
            "free_mem": free_mem,
            "swap_free": swap_free,
            "swap_cmm": swap_cmm,
            "eden_ini": eden_ini,
            "eden_cmm": eden_cmm,
            "eden_max": eden_max,
            "eden_used": eden_used,
            "svv_ini": svv_ini,
            "svv_cmm": svv_cmm,
            "svv_max": svv_max,
            "svv_used": svv_used,
            "tnd_ini": tnd_ini,
            "tnd_cmm": tnd_cmm,
            "tnd_max": tnd_max,
            "tnd_used": tnd_used,
            "pm_ini": pm_ini,
            "pm_cmm": pm_cmm,
            "pm_max": pm_max,
            "pm_used": pm_used,
            "queue_size": queue_size,
            "consumers": consumers,
            "msg_ack": msg_ack,
            "msg_exp": msg_exp,
        }

        json.dump(action_data, bulk_json)
        bulk_json.write('\n')

        json.dump(sys_data, bulk_json)
        bulk_json.write('\n')

        i += 1

        if (i % 1000) == 0:
            if not quiet:
                sys.stdout.write("Bulk uploading sender network data (%d records out of %d)\r" % (i, num_lines))

            call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

            bulk_json.truncate(0)
            bulk_json.seek(0)

    call_service(req_url, bulk_json.getvalue(), session=session, is_update=True)

    if not quiet:
        print ""

    if skipped > 0:
        logger.warn("Skipped invalid %d records", skipped)

    datafile.close()
    bulk_json.close()

    configure_cache(session=session)
    return 0

def main():
    config_file = in_opts["config"]
    test_config_file = in_opts["config_test"]

    if config_file is not None and test_config_file is None:
        config.read(config_file)

    if config_file is None and test_config_file is not None:
        config.read(test_config_file)

    if config_file is not None and test_config_file is not None:
        config.read( [config_file, test_config_file])

    is_register = in_opts["register"]

    if is_register:
        return register()
    else:
        is_test_info = in_opts["testinfo"]

        if is_test_info:
            return load_test_info()

        else:
            in_direction = in_opts["msg_direction"]
            in_load = in_opts["load"]

            if in_direction is None:
                logger.error("Direction is required")

                return 1

            if in_load is None:
                logger.error("Load data type is required (either latency or throughput)")

                return 1

            if in_direction == "receiver" and in_load == "latency":
                return load_latencies_bulk()

            if in_direction == "receiver" and in_load == "throughput":
                return load_throughput_bulk()

            if in_direction == "sender" and in_load == "throughput":
                return load_throughput_bulk()

            if in_direction == "sender" and in_load == "network":
                return load_sender_network_info()

            if in_direction == "receiver" and in_load == "java":
                return load_broker_java_info()


    return

# main() call
# ---------------------------------------------------------------------------

if __name__ == "__main__":
    # parameters definition and parsing
    # -------------------------------------------------------------------------
    usage_msg = "usage: %prog [options]";
    op = optparse.OptionParser(usage=usage_msg);

    op.add_option("--config", dest="config", type="string",
                  action="store", help="Path to the configuration file",
                  metavar="CONFIG_FILE");

    op.add_option("--config-test", dest="config_test", type="string",
                  action="store", help="Path to the test configuration file",
                  metavar="CONFIG_TEST_FILE");

    # General options
    op.add_option("--url", dest="url", type="string",
                  action="store", help="Database server URL", metavar="URL");

    op.add_option("--filename", dest="filename", type="string",
                  action="store", default=None, metavar="FILENAME",
                  help="The input filename (def: %default)");

    op.add_option("--username", dest="username", type="string",
              action="store", default="admin", metavar="USERNAME",
              help="Database user name (def: %default)");

    op.add_option("--password", dest="password", type="string",
              action="store", default=None, metavar="PASSWORD",
              help="Database user password (def: %default)");

    op.add_option("--update", dest="update", action="store_true", default=False,
                  help="Update the record instead of creating a new one");

    # Actions
    op.add_option("--register", dest="register", action="store_true", default=False,
                  help="Register SUTs");

    op.add_option("--load", dest="load", type="string",
                  action="store", default=None, metavar="DATA_TYPE",
                  help="The test data (latency, throughput, network, java) to load into the DB (def: %default)");

    op.add_option("--testinfo", dest="testinfo", action="store_true", default=False,
                  help="Load test information data into the DB");

    op.add_option("--quiet", dest="quiet", action="store_true", default=False,
                  help="Do not display any output");

    # SUT stuff
    op.add_option("--sut-name", dest="sut_name", type="string",
                  action="store", default=None, metavar="SUT_NAME",
                  help="Software under test (SUT) name (def: %default)");

    op.add_option("--sut-key", dest="sut_key", type="string",
                  action="store", default=None, metavar="KEY",
                  help="Unique key identifier (ie: amq6, artemis, etc) for the SUT (def: %default)");

    op.add_option("--sut-version", dest="sut_version", type="string",
                  action="store", default=None, metavar="VERSION",
                  help="Version of the SUT");

    # Test specific stuff
    op.add_option("--test-run", dest="test_run", type="string",
                  action="store", default=None, metavar="TEST_RUN",
                  help="The test execution number (def: %default)");

    op.add_option("--test-duration", dest="test_duration", type="string",
                  action="store", default=None, metavar="DURATION",
                  help="The test duration in number of messages or time in the format " \
                       "<value><t> (ie: 1 day = 1d, 2 hours = 2h, etc)");

    op.add_option("--test-start-time", dest="test_start_time", type="string",
                  action="store", default=None, metavar="YYY-MM-DD HH:mm:ss",
                  help="The start time for the test (def: %default)");

    op.add_option("--test-comment", dest="test_comment", type="string",
                  action="store", default=None, metavar="COMMENT",
                  help="Test comment (def: %default)");

    op.add_option("--test-result-comment", dest="test_result_comment", type="string",
              action="store", default=None, metavar="RESULT_COMMENT",
              help="Test result comment (def: %default)");


    # Message stuff
    op.add_option("--msg-protocol", dest="msg_protocol", type="string",
                  action="store", default=None, metavar="PROTOCOL",
                  help="The messaging protocol used on the test [stomp, amqp, mqtt, etc] "\
                          "(def: %default)");

    op.add_option("--msg-size", dest="msg_size", type="string",
                  action="store", default=None, metavar="SIZE",
                  help="The message size used in the tests (def: %default / 0 for random)");

    op.add_option("--msg-direction", dest="msg_direction", type="string",
                  action="store", default=None, metavar="TYPE  ",
                  help="Message direction where it was flowing from/to/through (ie: receiver, sender, router)");

    op.add_option("--msg-endpoint-type", dest="msg_endpoint_type", type="string",
                  action="store", default="queue", metavar="TYPE",
                  help="The endpoint type (queue or topic)");

    # Consumer stuff
    op.add_option("--consumers-count", dest="consumer_count", type="string",
                  action="store", default=None, metavar="NUMBER",
                  help="The number of concurrent consumers (def: %default)");

    op.add_option("--consumer-sys-info", dest="consumer_sys_info", type="string",
                  action="store", default=None, metavar="CON_SYS_INFO",
                  help="The consumer system information (def: %default)");

    # Producer stuff

    op.add_option("--producers-count", dest="producer_count", type="string",
                  action="store", default=None, metavar="NUMBER",
                  help="The number of concurrent producers (def: %default)");

    op.add_option("--producer-sys-info", dest="producer_sys_info", type="string",
                  action="store", default=None, metavar="PROD_SYS_INFO",
                  help="The producer system information (def: %default)");

    # Broker system stuff
    op.add_option("--broker-sys-os-type", dest="broker_os_type", type="string",
                  action="store", default=None, metavar="OS_TYPE",
                  help="The host OS type [linux, windows, etc] (def: %default)");

    op.add_option("--broker-sys-os-name", dest="broker_os_name", type="string",
                  action="store", default=None, metavar="OS_NAME",
                  help="The host OS name [rhel, fedora, windows, etc] (def: %default)");

    op.add_option("--broker-sys-os-version", dest="broker_os_version", type="string",
                  action="store", default=None, metavar="OS_VERSION",
                  help="The host OS version [6.6, 6.7, 2012, etc] (def: %default)");

    op.add_option("--broker-sys-hw-type", dest="broker_hw_type", type="string",
                  action="store", default=None, metavar="HW_TYPE",
                  help="The hardware type [kvm, bare, vmware] (def: %default)");

    op.add_option("--broker-sys-info", dest="broker_sys_info", type="string",
                  action="store", default=None, metavar="BROKER_SYS_INFO",
                  help="The broker system information (def: %default)");



    (opts, args) = op.parse_args();


#    in_opts = {};
    in_opts = eval('%s' % opts);

    sys.exit(main());
