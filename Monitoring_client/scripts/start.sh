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

usage() {
    cat <<EOF
Usage: $0 [a:t:h]

-h                  prints this usage information.

-a <APPLICATION_ID> sets a user-defined application ID [optional]
-t <TASK_ID>        sets a user-defined task (component) ID [optional]

EOF
}

BASE_DIR=`pwd`/..
DIST_DIR=${BASE_DIR}/dist
DIST_BIN_DIR=${DIST_DIR}/bin
LIB_DIR=${DIST_DIR}/lib

PARAMS=''

while getopts "a:t:h" opt; do
  case $opt in
    a)
      PARAMS+=" -a ${OPTARG}"
      ;;
    t)
      PARAMS+=" -t ${OPTARG}"
      ;;
    h)
      usage
      exit 0
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      exit 1
      ;;
    :)
      echo "Option -$OPTARG requires an argument." >&2
      exit 1
      ;;
  esac
done;

# set environment variables
libs=${LIB_DIR}
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$libs

# run agent with given parameters
# nohup makes your process running even after you logout
nohup ${DIST_BIN_DIR}/mf_client ${PARAMS} 2>&1&
echo "mf_client started.";

# end

