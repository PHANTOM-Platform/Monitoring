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
 
# GLOBAL VARIABLES
	SERVER_DIR=~/phantom_servers/;
	BASE_DIR=`dirname $0`;
	TMP_DIR=${SERVER_DIR}/tmp;
	DIST_DIR=${SERVER_DIR}/dist;
# CHECK INSTALLATION
	mkdir -p ${TMP_DIR};
	echo "Checking ...";
	echo " > elasticsearch";
	ES_HOME=${DIST_DIR}/elasticsearch;
	ELASTICSEARCH_BIN=${ES_HOME}/bin/elasticsearch;
	cp ${BASE_DIR}/elasticsearch.yml ${ES_HOME}/config/elasticsearch.yml;
	elasticsearch_port="9400";
	nodejs_port=3033;

	command -v ${ELASTICSEARCH_BIN} >/dev/null 2>&1 || { echo " elasticsearch : Not installed. Aborting." >&2; exit 1; }

	RESULT=$(netstat -lnt | awk '$6 == "LISTEN" && $4 ~ ":${elasticsearch_port}"');
	if [[ -z "${RESULT// }" ]] ; then
		nohup ${ELASTICSEARCH_BIN} >/dev/null 2>&1 &
		echo $! > ${TMP_DIR}/elasticsearch.pid ;
	else
		echo -e " > port ${elasticsearch_port} already bound by another process. We assume that Elasticsearch is already running.\n";
	fi;

	let "j=0";
	HTTP_STATUS=$(curl -s -w %{http_code} http://localhost:${elasticsearch_port});

	while [[ ${HTTP_STATUS} != *"200"* ]] && [ ${j} -lt 30 ] ; do
		echo -n "$j. "; let "j += 1 ";  sleep 1;
		HTTP_STATUS=$(curl -s -w %{http_code} http://localhost:${elasticsearch_port});
	done;
	if [ ${j} -ge 30 ]; then 
		echo -e "[ERROR]: ElasticSearch doesn't started.\n"
		exit;
	fi;

	echo ;
	if [[ ${HTTP_STATUS} != *"200"* ]]; then
		echo -e "> Elasticsearch is unreachable. Aborting.\n"
		exit 1;
	fi;
	echo -e "Done. Elasticsearch started successfully on port 9400.\n";

	echo "Starting the monitoring server ...";
	NODE_DIR=${DIST_DIR}/nodejs/bin;
	NODE_BIN=${NODE_DIR}/node;
	NPM_BIN=${DIST_DIR}/nodejs/bin/npm;
	export PATH=${NODE_DIR}:${PATH};

	command -v ${NODE_BIN} >/dev/null 2>&1 || { echo " node  : Not installed. Aborting.\n" >&2; exit 1; }
	command -v ${NPM_BIN} >/dev/null 2>&1 || { echo " npm  : Not installed. Aborting.\n" >&2; exit 1; }
	if [ ! -e node_modules ]; then
		ln -s ~/phantom_servers/node_modules node_modules;
	fi;
	nohup ${DIST_DIR}/nodejs/bin/node ./bin/www >/dev/null 2>&1 &
	echo $! > ${TMP_DIR}/monitoring.pid;
	echo -e "Done. Server is listening on port ${nodejs_port}.\n";
