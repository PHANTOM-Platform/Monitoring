#!/bin/bash
#  Copyright (C) 2016-2018 University of Stuttgart
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
	SERVER_DIR=~/phantom_servers; 
	BASE_DIR=`dirname $0`;
	cd ${BASE_DIR};
	BASE_DIR=`pwd`;
	TMP_DIR=${SERVER_DIR}/tmp;
	DIST_DIR=${SERVER_DIR}/dist;
	if [ ! -e ${SERVER_DIR} ]; then
		mkdir ${SERVER_DIR};
	fi;
	if [ ! -e ${TMP_DIR} ]; then
		mkdir ${TMP_DIR};
	fi;
	if [ ! -e ${DIST_DIR} ]; then
		mkdir ${DIST_DIR};
	fi;
# SOFTWARE
	NODE_JS_VERSION="9.4.0";
	NODE_JS="node-v${NODE_JS_VERSION}-linux-x64";
# REQUIRED SOFTWARE CHECKS
# > git and wget
	echo "Checking for required software:";
	echo "> git";
	command -v git >/dev/null 2>&1 || { echo " git  : Not installed. Aborting." >&2; exit 1; }
	echo "> wget";
	command -v wget >/dev/null 2>&1 || { echo " wget  : Not installed. Aborting." >&2; exit 1; }
	echo -e "Done.\n"; 
# DOWNLOADING AND INSTALLING EXTERNAL DEPENDENCIES
# > node.js and npm
	cd ${TMP_DIR};
	echo -e "Installing external dependencies:\n > node.js";
	if [ ! -f "${NODE_JS}.tar.gz" ]; then 
		wget https://nodejs.org/dist/v${NODE_JS_VERSION}/${NODE_JS}.tar.gz;
	fi;
	if [ ! -d "${DIST_DIR}/${NODE_JS}" ]; then 
		tar -xf ${NODE_JS}.tar.gz;
		rm -fr ${DIST_DIR}/nodejs;
		mv ${NODE_JS} ${DIST_DIR}/nodejs;
	fi;
	sync;
# INSTALL NODE AND NPM and setup the environment
	cd ${SERVER_DIR};
	cp ${BASE_DIR}/package.json .;
	NODE_DIR=${DIST_DIR}/nodejs/bin;
	NODE_BIN=${NODE_DIR}/node;
	NPM_BIN=${DIST_DIR}/nodejs/bin/npm;
	export PATH=${NODE_DIR}:${PATH};
	command -v ${NODE_BIN} >/dev/null 2>&1 || { echo " node  : Not installed. Aborting." >&2; exit 1; }
	command -v ${NPM_BIN} >/dev/null 2>&1 || { echo " npm  : Not installed. Aborting." >&2; exit 1; }
	${NPM_BIN} install;
	echo -e "Done.\n";

