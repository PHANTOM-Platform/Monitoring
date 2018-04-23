#!/bin/bash
#  Copyright (C) 2018 University of Stuttgart
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
# STOPPING SERVICES
	echo "Stopping services ...";
	echo " > elasticsearch";
	if [ -f "${TMP_DIR}/elasticsearch.pid" ]; then
	    PID=$(cat ${TMP_DIR}/elasticsearch.pid) ;
	    kill ${PID};
	    rm -f ${TMP_DIR}/elasticsearch.pid;
	else
	    echo "Couldn't find PID associated with elasticsearch process.";
	    echo "Please kill the service manually.";
	fi;

	echo " > node";
	if [ -f "${TMP_DIR}/monitoring.pid" ]; 	then
	    PID=$(cat ${TMP_DIR}/monitoring.pid);
	    kill ${PID};
	    rm -f ${TMP_DIR}/monitoring.pid;
	else
	    echo "Couldn't find PID associated with the monitoring node process.";
	    echo "Please kill the service manually.";
	fi;
	echo -e "Done.\n";

