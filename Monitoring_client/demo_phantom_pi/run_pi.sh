#!/bin/bash

if [ $# -lt 1 ]; then 
	echo "missing input parameter exec_idw";
	exit;
fi;

# input parameter is the execution_id which is a text string.
execution_id=${1};
token="eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9.eyJzdWIiOiJtb250YW5hQGFiYy5jb20iLCJpYXQiOjE1NDc2Mjg5MTQsImV4cCI6MTU1MDIyMDkxNH0.Iy4wJT7gPfN2moW2qplnrAmVu22rdT4T0PXahL-_Ccs";
# Delete the data from the previous experiment for more easy inspection
curl -s -XDELETE http://localhost:9400/montanana_demo_pthread-example > /dev/null;
echo -e "\n\n\n";
if [ -e exec_stats.json ];then rm exec_stats.json; fi;
if [ -d log ]; then rm -fr log; fi;

common_path="/home/jmontana/phantom_mf/Monitoring_client";
export LD_LIBRARY_PATH=${common_path}/bin/curl/lib:${common_path}/src/publisher:${common_path}/src/api:$LD_LIBRARY_PATH;
export LD_LIBRARY_PATH=${common_path}/dist/lib:$LD_LIBRARY_PATH;
./bin/pi_simple ${execution_id} ${token} 2000 2000;
