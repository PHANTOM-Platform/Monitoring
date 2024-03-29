# PHANTOM monitoring server

> PHANTOM monitoring server is one part of the PHANTOM monitoring framework, which supports monitoring of performance and power metrics for heterogeneous platforms and hardwares. 


## Introduction
The PHANTOM monitoring server is composed of two components: a web server and a data storage system. The web server provides various functionalities for data query and data analysis via RESTful APIs with documents in JSON format. The server's URL are "http://localhost:3033" by default.


 <p align="center">
<a href="https://github.com/PHANTOM-Platform/Monitoring/blob/master/Monitoring_server/d2.2.mf-server_v2.png">
<img src="https://github.com/PHANTOM-Platform/Monitoring/blob/master/Monitoring_server/d2.2.mf-server_v2.png" align="middle" width="70%" height="70%" title="Schema" alt="MF Server+ Execution Manager Schema">
</a> </p>


## Prerequisites
The monitoring server receives data from the [PHANTOM monitoring client][client] via RESTful interfaces. The server is implemented using Node.js, and connects to Elasticsearch to store and access metric data. Before you start installing the required components, please note that the installation and setup steps mentioned below assume that you are running a current Linux as operating system. The installation was tested with Ubuntu 16.04 LTS as well as with Scientific Linux 6 (Carbon).
Before you can proceed, please clone the repository:

```bash
svn export https://github.com/PHANTOM-Platform/Monitoring.git/trunk/Monitoring_server Monitoring_server
```



### Dependencies
This project requires the following dependencies to be installed:

| Component         | Homepage                                           | Version   |
|------------------ |--------------------------------------------------  |---------  |
| Elasticsearch     | https://www.elastic.co/products/elasticsearch      | = 2.4.6  |
| Node.js           | https://apr.apache.org/                            | >= 4.5    |
| npm               | https://www.npmjs.com/                             | >= 1.3.6  |
 

#### Installation of npm
When using Ubuntu, for example, please install npm as follows:

```bash
sudo apt-get install npm
```

Alternativelly, you can install it using your operating system's software installer.

## Installation of other components
This section assumes that you've successfully installed all required dependencies as described in the previous paragraphs.

#### 1.- To ease the installation and preparation process, there is one shell script provided, which downloads and installs all the dependencies and packages. 
Installs Nodejs 9.4.0. and Elastic-Search 2.4.6 on the folder {your_local_home_folder}/phantom_server.
Please choose the appropriate shell scripts depending on your Operating System:


Shell script for Intel-x86 32bits (tested on Ubuntu):

```bash
bash setup-server-x86-32.sh
```

or the Shell script for Intel-x86 64bits (tested on Ubuntu):

```bash
bash setup-server-x86-64.sh
```

or the Shell script for Armv7l 64bits (tested on Raspbian):

```bash
bash setup-server-armv7-64.sh
```

The default port is 3033.

This setup provides the configuration to the elastic-search by placing in the appropiate folder the file elasticsearch.yml (from this repository). That configuration file makes that `Elasticsearch` uses the port 9400 instead of the default port 9200.

#### 2.- The PHANTOM Repository relies on the Elasticsearch running on the SAME server, which should be installed by the previous scripts.


Please take a look on the next suggested reference books, if you face difficulties on the setup of ElasticSearch-Database server: 


* [Elasticsearch in Action][Elasticsearch in Action]
* [Elasticsearch Essentials][Elasticsearch Essentials]
* [Elasticsearch Server][Elasticsearch Server]
* [Elasticsearch: The Definitive Guide][Elasticsearch: The Definitive Guide]
* [Elasticsearch Cookbook][Elasticsearch Cookbook]



## Start/Stop the server
Start a Elasticsearch database and then the monitoring web server by executing, it is important to not do as root:
For security reasons, the services will not start if they are requested from root.

```bash
bash start-monitoring-server.sh
```

You can use the following commands to verify if the database and the server are running

Test of the Elastic Search running service:

```bash
curl http://localhost:9400
```


Test of the Nodejs Front-end running service:

```bash
curl http://localhost:3033
```



After the usage, the server can be stopped by:
```bash
bash stop-monitoring-server.sh
```


## RESTful API Queries
It follows a list of some RESTful queries to demonstrate its usage:

```bash
# WORKFLOWS
GET  /v1/phantom_mf/workflows
GET  /v1/phantom_mf/workflows/:application_id
PUT  /v1/phantom_mf/workflows/:application_id -d '{...}'

# EXPERIMENTS
GET  /v1/phantom_mf/experiments
GET  /v1/phantom_mf/experiments/:execution_id?workflow=:application_id
POST /v1/phantom_mf/experiments/:application_id -d '{...}'

# METRICS
GET  /v1/phantom_mf/metrics/:application_id/:task_id/:execution_id
POST /v1/phantom_mf/metrics -d '{...}'
POST /v1/phantom_mf/metrics/:application_id/:task_id/:execution_id -d '{...}'

# PROFILES
GET /v1/phantom_mf/profiles/:application_id
GET /v1/phantom_mf/profiles/:application_id/:task_id
GET /v1/phantom_mf/profiles/:application_id/:task_id/:execution_id
GET /v1/phantom_mf/profiles/:application_id/:task_id/:execution_id?from=...&to=...

# RUNTIME
GET /v1/phantom_mf/runtime/:application_id/:execution_id
GET /v1/phantom_mf/runtime/:application_id/:task_id/:execution_id

# STATISTICS
GET /v1/phantom_mf/statistics/:application_id?metric=...
GET /v1/phantom_mf/statistics/:application_id?metric=...&host=...
GET /v1/phantom_mf/statistics/:application_id?metric=...&from=...&to=...
GET /v1/phantom_mf/statistics/:application_id?metric=...&host=...&from=...&to=...
GET /v1/phantom_mf/statistics/:application_id/:execution_id?metric=...
GET /v1/phantom_mf/statistics/:application_id/:execution_id?metric=...&host=...
GET /v1/phantom_mf/statistics/:application_id/:execution_id?metric=...&from=...&to=...
GET /v1/phantom_mf/statistics/:application_id/:execution_id?metric=...&host=...&from=...&to=...
GET /v1/phantom_mf/statistics/:application_id/:task_id/:execution_id?metric=...
GET /v1/phantom_mf/statistics/:application_id/:task_id/:execution_id?metric=...&host=...
GET /v1/phantom_mf/statistics/:application_id/:task_id/:execution_id?metric=...&from=...&to=...
GET /v1/phantom_mf/statistics/:application_id/:task_id/:execution_id?metric=...&host=...&from=...&to=...

# RESOURCES (Resource Manager)
GET  /v1/phantom_rm/resources
GET  /v1/phantom_rm/resources/:platform_id
PUT  /v1/phantom_rm/resources/:platform_id -d '{...}'

# CONFIGS (Resource Manager)
GET  /v1/phantom_rm/configs
GET  /v1/phantom_rm/configs/:platform_id
PUT  /v1/phantom_rm/configs/:platform_id -d '{...}'
```

Please refer to the [Reference Manual of the RESTful API][apimanual] to get more details.
The Manual contains a description of each of the implemented methods, with examples of use, and type of possible responses. 


## Acknowledgment
This project is realized through [EXCESS][excess] and [PHANTOM][phantom]. EXCESS is funded by the EU 7th Framework Programme (FP7/2013-2016) under grant agreement number 611183. The PHANTOM project receives funding under the European Union's Horizon 2020 Research and Innovation Programme under grant agreement number 688146.


## Contributing
Find a bug? Have a feature request?
Please [create](https://github.com/hpcfapix/phantom_monitoring_server/issues) an issue.


## Main Contributors

**Montanana, Jose Miguel, HLRS**
+ [github/jmmontanana](https://github.com/jmmontanana)

**Cheptsov, Alexey, HLRS**
+ [github/alexey-cheptsov](https://github.com/alexey-cheptsov)
 

## Release History
| Date        | Version | Comment          |
| ----------- | ------- | ---------------- |
| 2017-03-28  | 1.0     | First prototype  |
| 2018-01-23  | 1.01    | Current version  |

## License
Copyright (C) 2014-2018 University of Stuttgart

[Apache License v2](LICENSE).
 
[client]: https://github.com/PHANTOM-Platform/Monitoring/tree/master/Monitoring_client
[server]: https://github.com/PHANTOM-Platform/Monitoring/tree/master/Monitoring_server 
[phantom]: http://www.phantom-project.org
[api]: https://phantom-monitoring-framework.github.io
[apimanual]: https://phantom-platform.github.io/Monitoring/docs/

<b>For the updated version 2023 contact with jmmontanana at gmail.com</b>
