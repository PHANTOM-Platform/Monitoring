#!/bin/bash
#Script for seting up an new database, and register the users from the file list_of_users.ini;
#Author: J.M.Montañana HLRS 2018
# If you find any bug, please notify to hpcjmont@hlrs.de

#Copyright (C) 2018 University of Stuttgart
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
# 	http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# GLOBAL VARIABLES
	port=3033;
	server=localhost;
	expectedserver="PHANTOM Monitoring Server is up and running.";
	
	app=`basename $0`;
	SERVER_DIR=~/phantom_servers/;
	BASE_DIR=`dirname $0`;
	cd ${BASE_DIR};
	#Set up an new database, 
	#In case that already setup, it will not perform any change just only return an error on already existing DB. 
#0. #### Function Scripts definition ################################
	verify_reponse()
	{ 
		# $1 server
		# $2 port
		# $3 expectedserver
		echo "Checking Response on port ${2} ...";
		let "j=0"; 
		if [ "$#" -lt 3 ]; then
			echo "error missing parameters at function verify_response";
			exit 1;
		fi;
		HTTP_STATUS=$(curl --silent --output /dev/null --write-out "%{http_code}" http://${1}:${2});
		while [[ ${HTTP_STATUS} != "200" ]] && [ ${j} -lt 1 ] ; do 
			let "j += 1 "; sleep 1; 
			HTTP_STATUS=$(curl --silent --output /dev/null --write-out "%{http_code}" http://${1}:${2});
		done; 
		if [[ ${HTTP_STATUS} != "200" ]]; then
			echo "> Server is unreachable on port ${2}. Aborting.";
			exit 1;
		fi;

		HTTP_STATUS=$(curl -s http://${1}:${2}/verify_es_connection);
		if [[ ${HTTP_STATUS} != "200" ]]; then
			echo "> Server has not connection with the ElasticSearch server. Aborting.";
			exit 1;
		fi;
		
		# Look which kind of server is listening
		SERVERNAME=$(curl --silent http://${1}:${2}/servername);
		if [[ ${SERVERNAME} != ${3} ]]; then
			echo " The server found is not a ${3} server. Aborting.";
			echo ${SERVERNAME};
			exit 1;			
		fi;
		echo "Done. Response successfully found on port ${2}.";
		echo ;
	}
# 1. ################# CHECK if the server is running ###############
	echo "Checking ${expectedserver} server ...";
	verify_reponse ${server} ${port} "${expectedserver}"; 
 
# 3.- REGISTER NEW USERS
	while read -r line ; do
		line=${line%%#*} # strip comment (if any)
		fname=${line#\"};
		name=${fname%%\"*};
		name=${name// /%20};
		params=${fname#*\"}
		params=${params#*	}
		user=${params%%	*};
		userpw=${params#*	};#remove ending tabs
		userpw=${userpw%% };#remove ending spaces
		password=${userpw%%	};#remove ending tabs
		echo "name    is: \"${name}\"";
		echo "user_id is: \"${user}\"";
		echo -e "password is: \"${password}\"\n";
		curl -s -H "Content-Type: application/json" -XPOST http://${server}:${port}/signup?name="${name}"\&email="${user}"\&pw="${password}";
		curl -s -XGET http://${server}:${port}/_flush > /dev/null;
	done <"list_of_users.ini";
	echo -e "done.\n";
exit
curl -s -H "Content-Type: application/json" -XPOST http://${server}:${port}/update_user?email="montana@abc.com"\&pw="124154151235";
