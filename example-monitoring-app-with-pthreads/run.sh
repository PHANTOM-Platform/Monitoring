#!/bin/bash

#if the initial.sh was never run before, then you need to run it:
#bash initial.sh


common_path="../phantom_monitoring_client"
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${common_path}/src/api
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${common_path}/src/parser
export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${common_path}/src/publisher
currentdate=`date '+%Y-%m-%dT%H:%M:%S.%3N'`; #echo $currentdate
#I take the currentdate as common id for all the treads, I suppose the current time can be an unique id different from other cases.
#It is needed because each thread will register its results as a different experiment
#in the DB will be stored multiple experiments from diferent threads, and we will need to know which correspond to this exection.

if [ -e bin/log.txt ]; then
	rm bin/log.txt;
fi;
./bin/pthread-example $currentdate  

#Notice when the program is executed a new folder is created inside the bin folder.
#The name of the folder is the name of the pid when executing the pthread-example
#Notice that the stored data in the elasticsearch is also copied in text files in such new folder.
#Please delete that folder if you need that copy, now is still not deleted for debugging purposes.

