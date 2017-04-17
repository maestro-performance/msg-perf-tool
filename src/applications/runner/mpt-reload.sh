#!/bin/bash

app_path=`dirname $0`

source $1

if [[ -z "${LOADER_CONFIG}" ]] ; then
  echo "Loader config was not informed, therefore skipping loading test data"

  return 0
fi
  

echo "Registering the SUT on the DB using ${LOADER_CONFIG}"
${app_path}/mpt-loader.py --register \
  --config "${LOADER_CONFIG}" \
  --config-test "${CONFIG_TEST}" \

echo "Registering the test case on the DB"
${app_path}/mpt-loader.py --testinfo \
  --config "${LOADER_CONFIG}" \
  --config-test "${CONFIG_TEST}" \
  --quiet \
  --test-run "${TEST_RUN}" \
  --test-start-time "${START_TIME}" \
  --test-duration "${REAL_DURATION}" \
  --test-comment "${TEST_NAME} small automated test case" \
  --test-result-comment "Run ok, no comments"

for file in ${LOG_DIR}/${TEST_RUN}/sender-throughput-*.csv ; do
  echo "Loading file: ${file}"
  ${app_path}/mpt-loader.py --load throughput \
    --config "${LOADER_CONFIG}" \
    --config-test "${CONFIG_TEST}" \
    --test-start-time "${START_TIME}" \
    --quiet \
    --test-run "${TEST_RUN}" \
    --msg-direction sender \
    --filename ${file}
done

for file in ${LOG_DIR}/${TEST_RUN}/receiver-throughput-*.csv ; do
  echo "Loading file: ${file}"
  ${app_path}/mpt-loader.py --load throughput \
    --config "${LOADER_CONFIG}" \
    --config-test "${CONFIG_TEST}" \
    --test-start-time "${START_TIME}" \
    --quiet \
    --test-run "${TEST_RUN}" \
    --msg-direction receiver \
    --filename ${file}
done

for file in ${LOG_DIR}/${TEST_RUN}/receiver-latency-*.csv ; do
  echo "Loading file: ${file}"
  ${app_path}/mpt-loader.py --load latency \
    --config "${LOADER_CONFIG}" \
    --config-test "${CONFIG_TEST}" \
    --test-start-time "${START_TIME}" \
    --quiet \
    --test-run "${TEST_RUN}" \
    --msg-direction receiver \
    --filename ${file}
done

for file in ${LOG_DIR}/${TEST_RUN}/network-statistics-*.csv ; do
  echo "Loading file: ${file}"
  ${app_path}/mpt-loader.py --load network \
    --config "${LOADER_CONFIG}" \
    --config-test "${CONFIG_TEST}" \
    --test-start-time "${START_TIME}" \
    --quiet \
    --test-run "${TEST_RUN}" \
    --msg-direction sender \
    --filename ${file}
done

for file in ${LOG_DIR}/${TEST_RUN}/broker-jvm-statistics-*.csv ; do
  echo "Loading file: ${file}"
  ${app_path}/mpt-loader.py --load java \
    --config "${LOADER_CONFIG}" \
    --config-test "${CONFIG_TEST}" \
    --test-start-time "${START_TIME}" \
    --quiet \
    --test-run "${TEST_RUN}" \
    --msg-direction receiver \
    --filename ${file}
done