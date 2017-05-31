#!/bin/bash
#  Copyright (C) 2016 University of Stuttgart
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
# GLOBAL VARIABLES
#
BASE_DIR=`pwd`
TMP_DIR=${BASE_DIR}/tmp
DIST_DIR=${BASE_DIR}/dist

#
# CHECK INSTALLATION
#
mkdir -p ${TMP_DIR}

echo "Checking ..."
echo "> elasticsearch"

ES_HOME=${DIST_DIR}/elasticsearch
ELASTICSEARCH_BIN=${ES_HOME}/bin/elasticsearch
cp ${BASE_DIR}/elasticsearch.yml ${ES_HOME}/config/elasticsearch.yml

command -v ${ELASTICSEARCH_BIN} >/dev/null 2>&1 || { echo " elasticsearch : Not installed. Aborting." >&2; exit 1; }

RESULT=$(netstat -lnt | awk '$6 == "LISTEN" && $4 ~ ":9400"')
if [[ -z "${RESULT// }" ]]
then
    nohup ${ELASTICSEARCH_BIN} >/dev/null 2>&1 &
    echo $! > ${TMP_DIR}/elasticsearch.pid
else
    echo "> port 9400 already bound by another process. We assume that Elasticsearch is already running."
fi

sleep 10

HTTP_STATUS=$(curl -s -w %{http_code} localhost:9400)
if [[ ${HTTP_STATUS} != *"200"* ]]
then
    echo "> Elasticsearch is unreachable. Aborting."
    exit 1;
fi
echo "Done. Elasticsearch started successfully on port 9400."
echo

echo "Starting the monitoring server ..."
NODE_DIR=${DIST_DIR}/nodejs/bin
NODE_BIN=${NODE_DIR}/node
NPM_BIN=${DIST_DIR}/nodejs/bin/npm
export PATH=${NODE_DIR}:${PATH}

command -v ${NODE_BIN} >/dev/null 2>&1 || { echo " node  : Not installed. Aborting." >&2; exit 1; }
command -v ${NPM_BIN} >/dev/null 2>&1 || { echo " npm  : Not installed. Aborting." >&2; exit 1; }

${NPM_BIN} install
nohup ./bin/www >/dev/null 2>&1 &
echo $! > ${TMP_DIR}/node.pid
echo "Done. Server is listening on port 3033."
echo
