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
# SOFTWARE
#
ELASTICSEARCH_VERSION="2.4.0"
ELASTICSEARCH="elasticsearch-${ELASTICSEARCH_VERSION}"
NODE_JS_VERSION="4.2.1"
NODE_JS="node-v${NODE_JS_VERSION}-linux-x64"

#
# SETUP
#
rm -rf ${DIST_DIR}
mkdir ${TMP_DIR}
mkdir ${DIST_DIR}

#
# REQUIRED SOFTWARE CHECKS
# > git
# > wget
#
echo "Checking for required software:"
echo "> git"
command -v git >/dev/null 2>&1 || { echo " git  : Not installed. Aborting." >&2; exit 1; }
echo "> wget"
command -v wget >/dev/null 2>&1 || { echo " wget  : Not installed. Aborting." >&2; exit 1; }
echo "Done."
echo

#
# DOWNLOADING AND INSTALLING EXTERNAL DEPENDENCIES
# > elasticsearch
# > node.js and npm
#
echo "Installing external dependencies:"
echo "> elasticsearch"
cd ${TMP_DIR}
if [ ! -f "${ELASTICSEARCH}.tar.gz" ]
then
    wget https://download.elasticsearch.org/elasticsearch/elasticsearch/${ELASTICSEARCH}.tar.gz
fi
if [ ! -d "${DIST_DIR}/${ELASTICSEARCH}" ]
then
    tar -xf ${ELASTICSEARCH}.tar.gz
    mv ${ELASTICSEARCH} ${DIST_DIR}/elasticsearch
fi

echo "> node.js"
cd ${TMP_DIR}
if [ ! -f "${NODE_JS}.tar.gz" ]
then
    wget https://nodejs.org/dist/v${NODE_JS_VERSION}/${NODE_JS}.tar.gz
fi

if [ ! -d "${DIST_DIR}/${NODE_JS}" ]
then
    tar -xf ${NODE_JS}.tar.gz
    mv ${NODE_JS} ${DIST_DIR}/nodejs
fi

rm -rf ${TMP_DIR}

sleep 10
#
# INSTALL NODE AND NPM and setup the environment
#
cd ${BASE_DIR}
NODE_DIR=${DIST_DIR}/nodejs/bin
NODE_BIN=${NODE_DIR}/node
NPM_BIN=${DIST_DIR}/nodejs/bin/npm
export PATH=${NODE_DIR}:${PATH}

command -v ${NODE_BIN} >/dev/null 2>&1 || { echo " node  : Not installed. Aborting." >&2; exit 1; }
command -v ${NPM_BIN} >/dev/null 2>&1 || { echo " npm  : Not installed. Aborting." >&2; exit 1; }

${NPM_BIN} install

echo "Done."
echo