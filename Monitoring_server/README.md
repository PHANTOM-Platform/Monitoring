# PHANTOM monitoring server

> PHANTOM monitoring server is one part of the PHANTOM monitoring framework, which supports monitoring of performance and power metrics for heterogeneous platforms and hardwares. 


## Introduction
The PHANTOM monitoring server is composed of two components: a web server and a data storage system. The web server provides various functionalities for data query and data analysis via RESTful APIs with documents in JSON format. The server's URL are "localhost:3033" by default.


## Prerequisites
The monitoring server receives data from the [PHANTOM monitoring client][client] via RESTful interfaces. The server is implemented using Node.js, and connects to Elasticsearch to store and access metric data. Before you start installing the required components, please note that the installation and setup steps mentioned below assume that you are running a current Linux as operating system. The installation was tested with Ubuntu 16.04 LTS as well as with Scientific Linux 6 (Carbon).
Before you can proceed, please clone the repository:

```bash
git clone https://github.com/hpcfapix/phantom_monitoring_server.git
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

To ease the installation and preparation process, there is one shell script provided, which downloads and installs all the dependencies and packages. Installs Elastic-Search 2.4.0 and Nodejs 9.4.0. Please choose the appropiate shell scripts depending on your Operating System :


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


This setup provides the configuration to the elastic-search by placing in the appropiate folder the file elasticsearch.yml (from this repository). That configuration file makes that `Elasticsearch` uses the port 9400 instead of the default port 9200.

 

## Start/Stop the server
Start a Elasticsearch database and then the monitoring web server by executing, it is important to not do as root:
For security reasons, the services will not start if they are requested from root.

```bash
bash start.sh
```

You can use the following commands to verify if the database and the server are running

Test of the Elastic Search running service:

```bash
curl localhost:9400
```


Test of the Nodejs Front-end running service:

```bash
curl localhost:9400
```



After the usage, the server can be stopped by:
```bash
./stop.sh
```


## RESTful Queries
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

Please refer to the [PHANTOM Monitoring API Web page][api] to get more details.


## Acknowledgment
This project is realized through [EXCESS][excess] and [PHANTOM][phantom]. EXCESS is funded by the EU 7th Framework Programme (FP7/2013-2016) under grant agreement number 611183. The PHANTOM project receives funding under the European Union's Horizon 2020 Research and Innovation Programme under grant agreement number 688146.


## Contributing
Find a bug? Have a feature request?
Please [create](https://github.com/hpcfapix/phantom_monitoring_server/issues) an issue.


## Main Contributors

**Montanana, Jose Miguel, HLRS**
+ [github/hpcfapix](https://github.com/jmmontanana)

**Cheptsov, Alexey, HLRS**
+ [github/hpcfapix](https://github.com/alexey-cheptsov)

**Fangli Pi, HLRS**
+ [github/hpcfapix](https://github.com/hpcfapix)


## Release History
| Date        | Version | Comment          |
| ----------- | ------- | ---------------- |
| 2017-03-28  | 1.0     | First prototype  |
| 2018-01-23  | 1.01    | Current version  |

## License
Copyright (C) 2014-2018 University of Stuttgart

[Apache License v2](LICENSE).


[client]: https://github.com/hpcfapix/phantom_monitoring_client
[excess]: http://www.excess-project.eu
[phantom]: http://www.phantom-project.org
[api]: https://phantom-monitoring-framework.github.io
