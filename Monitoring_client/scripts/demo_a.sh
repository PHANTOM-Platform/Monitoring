#!/bin/bash
#  Copyright (C) 2015 University of Stuttgart
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

#
# input parameters
#
APPLICATION_ID="dummy"
TASK_ID="t1"
MF_SERVER="141.58.0.8:3033"
METRIC_NAME="power_total"

#
# dummy application path and script
# 5: parallel computing 
# 4: number of threads is 4
#
DUMMY_PATH=/nas_home/hpcfapix/hpcfapix/measure/avx
DUMMY_CONFIG=/nas_home/hpcfapix/hpcfapix/measure/config.ini
DUMMY_PARAMS="${DUMMY_CONFIG} 5 4"

#
# mf_client path and ld_library_path setup
#
BASE_DIR=`pwd`/..
DIST_DIR=${BASE_DIR}/dist
DIST_BIN_DIR=${DIST_DIR}/bin
LIB_DIR=${DIST_DIR}/lib
LOG_DIR=${DIST_BIN_DIR}/log

rm ${LOG_DIR} -rf
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${LIB_DIR}

#
# mf_client parameters
# -a: application_id
# -t: task_id
#
PARAMS=" -a ${APPLICATION_ID} -t ${TASK_ID}"

#
# start mf_client
#
DATE_1=$(date +"%FT%T.%3N")
echo "Start mf_client at ${DATE_1}" 
${DIST_BIN_DIR}/mf_client ${PARAMS} 2>&1&

#
# run the dummy application
#
sleep 1
DATE_2=$(date +"%FT%T.%3N")
echo "Start application at ${DATE_2}"

${DUMMY_PATH} ${DUMMY_PARAMS} > /dev/null 2>&1

DATE_3=$(date +"%FT%T.%3N")
echo "Application ends at ${DATE_3}" 
sleep 1

#
# stop mf_client
#
DATE_4=$(date +"%FT%T.%3N")
echo "Stop mf_client at ${DATE_4}" 
PID=`ps -ef| grep -v grep | grep mf_client | awk '{print $2}'`
kill $PID

#
# wait for the log file is created
#
sleep 30

#
# read from logfile the experiment_id
#
EXPERIMENT_ID=`cat ${LOG_DIR}/* | grep experiment_id | awk '{print $6}'`

echo "Please try the following query for retrieving monitoring data and statistics:"
echo " "
echo "curl -XGET ${MF_SERVER}/v1/mf/profiles/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}"
echo "curl -XGET ${MF_SERVER}/v1/mf/runtime/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}"
echo "curl -XGET ${MF_SERVER}/v1/mf/statistics/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}?metric=${METRIC_NAME}"
echo "curl -XGET '${MF_SERVER}/v1/mf/statistics/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}?metric=${METRIC_NAME}&from=${DATE_2}&to=${DATE_3}'"

# end
