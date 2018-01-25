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
BASE_DIR=.
TMP_DIR=${BASE_DIR}/tmp

#
# SETUP
#
cd ${TMP_DIR}

#
# STOPPING SERVICES
#
echo "Stopping services ..."
echo "> elasticsearch"
if [ -f "elasticsearch.pid" ]
then
    PID=$(cat elasticsearch.pid)
    kill ${PID}
    rm -f elasticsearch.pid
else
    echo "Couldn't find PID associated with elasticsearch process."
    echo "Please kill the service manually."
fi

echo "> node"
if [ -f "node.pid" ]
then
    PID=$(cat node.pid)
    kill ${PID}
    rm -f node.pid
else
    echo "Couldn't find PID associated with node process."
    echo "Please kill the service manually."
fi
echo "Done."
echo