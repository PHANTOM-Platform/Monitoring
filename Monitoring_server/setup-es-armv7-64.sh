#!/bin/bash
# Copyright (C) 2018 University of Stuttgart
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This script was tested on a Raspeberri-Pi3 running Raspbian 4.9.28

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
	ELASTICSEARCH_VERSION="2.4.6";
	#DON'T USE YET ELASTICSEARCH_VERSION="5.5.1" it needs that the code/syntax of the services be modified
	ELASTICSEARCH="elasticsearch-${ELASTICSEARCH_VERSION}";
# REQUIRED SOFTWARE CHECKS
	echo "Checking for required software:";
	command -v wget >/dev/null 2>&1 || { echo " wget : Not installed. Aborting." >&2; exit 1; }
	echo -e "Done.\n"; 
# DOWNLOADING AND INSTALLING EXTERNAL DEPENDENCIES
# > elasticsearch
	echo -e "Installing external dependencies:\n > elasticsearch";
	cd ${TMP_DIR};
	if [ ! -f "${ELASTICSEARCH}.deb" ]; then
		wget https://download.elasticsearch.org/elasticsearch/release/org/elasticsearch/distribution/deb/elasticsearch/${ELASTICSEARCH_VERSION}/${ELASTICSEARCH}.deb
	fi;
	sudo dpkg -i ${ELASTICSEARCH}.deb
	#we need to add writtting permissions, because it ES has to run as user, and need to create some folders on the first run 
	sudo chmod 777 /usr/share/elasticsearch
	mkdir ${DIST_DIR}/elasticsearch
	mkdir ${DIST_DIR}/elasticsearch/bin
	ln -s /usr/share/elasticsearch/bin/elasticsearch ${DIST_DIR}/elasticsearch/bin/elasticsearch
	if [ ! -e "${DIST_DIR}/elasticsearch/config" ] ; then
		mkdir ${DIST_DIR}/elasticsearch/config
		ln -s ${DIST_DIR}/dist/elasticsearch/config /usr/share/elasticsearch/config
	fi;

