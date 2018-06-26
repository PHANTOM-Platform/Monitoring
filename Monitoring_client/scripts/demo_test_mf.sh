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
APPLICATION_ID="dummy" #should be same as given in the application
TASK_ID="t2" #should be same as given in the application
MF_SERVER="141.58.0.8:3033" #should be same as given in the application
METRIC_NAME="CPU_usage_rate"

#
# start the application with mf api
#
BASE_DIR=`pwd`/..
TEST_PATH=${BASE_DIR}/src/api/test
LIB_DIR=${BASE_DIR}/dist/lib
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:${LIB_DIR}

${TEST_PATH}/test_mf_api > demo_b_log.txt

sleep 10

#
# get experiment_id
#
EXPERIMENT_ID=`cat demo_b_log.txt | grep experiment_id | awk '{print $3}'`

echo "Please try the following query for retrieving monitoring data and statistics:"
echo " "
echo "curl -XGET ${MF_SERVER}/v1/mf/profiles/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}"
echo "curl -XGET ${MF_SERVER}/v1/mf/runtime/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}"
echo "curl -XGET ${MF_SERVER}/v1/mf/statistics/${APPLICATION_ID}/${TASK_ID}/${EXPERIMENT_ID}?metric=${METRIC_NAME}"
