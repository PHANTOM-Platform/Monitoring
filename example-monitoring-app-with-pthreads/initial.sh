#!/bin/bash
#for experiments with the pthread-example.c 
#It needs to be executed ONLY one time, to register the device and the workflow.

register_device()
{
	# $1 is the server
	# $2 is the server port
	# $3 is the regplatformid
	echo -e "\n\nREGISTERING DEVICE";
	curl -H "Content-Type: application/json" -XPUT http://$1:$2/v1/phantom_mf/devices -d '{ 
		"name" : "'"$3"'",
		"type" : "intel_x86-64", "total_cores" : 4,
		"local_timestamp" : "2017-06-01T07:51:06.325",
		"CPU_usage_rate" : 0.62, "RAM_usage_rate" : 26.351,
		"swap_usage_rate" : 0.0, "net_throughput" : 284.995,
		"io_throughput" : 229.593, "server_timestamp" : "2017-06-01T08:51:23.223"
	}'

	echo -e "\n\nREGISTERING CHARACTERIZATION OF ENERGY CONSUMPTION OF THE DEVICE";
	curl -H "Content-Type: application/json" -XPUT $1:$2/v1/phantom_rm/configs/$3 -d '{
		"parameters":{"MAX_CPU_POWER":"24.5","MIN_CPU_POWER":"6.0",
		"MEMORY_POWER":"2.016","L2CACHE_MISS_LATENCY":"59.80","L2CACHE_LINE_SIZE":"128","E_DISK_R_PER_KB":"0.0556",
		"E_DISK_W_PER_KB":"0.0438","E_NET_SND_PER_KB":"0.14256387","E_NET_RCV_PER_KB":"0.24133936"}
	}'
}

register_workflow()
{
	# $1 is the server
	# $2 is the server port
	# $3 is the component name
	# $4 is the appid
	# $5 is the task
	echo -e "\n\nREGISTERING WORKFLOW";
	curl -H "Content-Type: application/json" -XPUT $1:$2/v1/phantom_mf/workflows/$4 -d '{
		"application":"'"$4"'","author":"Random Guy","optimization":"Time","tasks":[{"name":"'"$3"'","exec":"'"$5"'","cores_nr": "1"}]}'
	echo -e "\n";

	# $6 is the regplatformid
	#echo -e "\nRegistering an new experiment\n";
	#curl -H "Content-Type: application/json" -XPOST $1:$2/v1/phantom_mf/experiments/$4 -d '{
	#	"application": "'"$4"'", "task": "'"$5"'", "host": "'"$6"'"}'
	#echo -e "\n\n";
}

#bash 0a-newmapping.sh ; #only if need to delete the database and create a new one

#server="141.58.5.218"  #instead of localhost can be specified the address of a remote server
server="localhost"
regplatformid="node01"
appid="demo"
task="pthread-example"
port="3033"

#curl -X DELETE localhost:9400/${appid}_${task} ; #in case we wish to delete all the previous experiments of this workflow

register_device ${server} ${port} ${regplatformid};

register_workflow ${server} ${port} main ${appid} ${task};
register_workflow ${server} ${port} update_ship_report  ${appid} ${task};

