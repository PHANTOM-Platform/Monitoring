#!/bin/bash
# Copyright (C) 2018 University of Stuttgart
#GLOBAL VARIABLES
	BASE_DIR=`dirname $0`;
	cd ${BASE_DIR};
	cd server_code;
# IF THE SERVER WAS RUNNING, WE STOP IT BEFORE START A NEW INSTANCE
	bash stop-monitoring-server.sh;
	bash start-monitoring-server.sh;
