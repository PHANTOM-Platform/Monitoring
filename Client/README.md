# PHANTOM monitoring client

> PHANTOM monitoring client is one part of the PHANTOM monitoring framework, which supports metrics collection based on hardware availabilities and platform configuration. The target platform of the monitoring client includes CPUs, GPUs, Myriad2, ACME power measurement kit and FPGA-based platform.


## Introduction
The essential functionality of PHANTOM monitoring client is metrics collection, which is implemented by various plug-ins according to the hardware accessibility.

Currently 7 plug-ins are supported, whose implementation and design details are collected in the directory `src/plugins`. The monitoring client is designed to be pluggable. Loading a plug-in means starting a thread for the specific plug-in based on the users’ configuration at run-time. The folder `src/agent` plays a role as the main control unit, as managing various plug-ins with the help of pthreads. Folders like `src/core`, `src/parser`, and `src/publisher` are used by the main controller for accessorial support, including parsing input configuration file (`src/mf_config.ini`), publishing metrics via HTTP, and so on. 

For code instrumentation and user-defined metrics collection, we provide a monitoring library and several APIs, which are kept in the directory `src/api`. Descriptions in detail about how to use the monitoring APIs are given also in this directory.


## Prerequisites
The monitoring client requires at first a running server and database. In order to install these requirements, please
checkout the associated [PHANTOM monitoring server][server] and follow the setup instructions given in the repository `README.md` file. A successful setup process can be checked by the following command as testing whether the server is running in the specific url:
```bash
curl localhost:3033
```

Please note that the installation and setup steps mentioned below assume that you are running a current Linux as operating system. We have tested the monitoring agent with Ubuntu 14.04 LTS as well as with Scientific Linux 6 (Carbon).

Before you can proceed, please clone the repository:
```bash
git clone https://github.com/hpcfapix/phantom_monitoring_client.git
```


### Dependencies
This project requires the following dependencies to be installed:

| Component         | Homepage                                          | Version   |
|------------------ |-------------------------------------------------  |---------  |
| PAPI-C            | http://icl.cs.utk.edu/papi/                       | 5.4.0     |
| CURL              | http://curl.haxx.se/download/                     | 7.37.0    |  
| Apache APR        | https://apr.apache.org/                           | 1.5.1     |
| Apache APR Utils  | https://apr.apache.org/                           | 1.5.3     |
| Nvidia GDK        | https://developer.nvidia.com/gpu-deployment-kit/  | 352.55    |
| bison             | http://ftp.gnu.org/gnu/bison/                     | 2.3       |
| flex              | http://prdownloads.sourceforge.net/flex/          | 2.5.33    |
| sensors           | https://fossies.org/linux/misc/                   | 3.4.0     |
| m4                | https://ftp.gnu.org/gnu/m4                        | 1.4.17    |
| libiio            | https://github.com/analogdevicesinc/libiio.git    | 1.0       |
| hwloc             | https://www.open-mpi.org/software/hwloc/v1.11/downloads/      | 1.11.2        |
| EXCESS queue      | https://github.com/excess-project/data-structures-library.git | release/0.1.0 |

To ease the process of setting up a development environment, we provide a basic script that downloads all dependencies, installs them locally in the project directory, and then performs some clean-up operations. Thus, compiling the monitoring client can be performed in a sandbox without affecting your current operating system.

Executing the following script
```bash
./setup.sh
```
results in a new directory named `bin`, which holds the required dependencies for compiling the project.


## Installation
This section assumes that you've successfully installed all required dependencies as described in the previous paragraphs.

```bash
make clean-all
make all
make install
```

The above commands clean, compile and install the monitoring agent into the directory `dist` within the project's repository. The `dist` folder includes all required binaries, shared libraries, scripts, and configuration files to get you started. The Makefile has been tested with GNU compiler version 4.9.2.


## Start monitoring
If you haven't yet followed our guide to set up the associated monitoring server and database, please do so now before continuing. Next, start the monitoring client with a default set of plugins enabled to monitor as follows:
```bash
cd scripts
./start.sh
```

You can learn more about various options passed to the monitoring client by calling
```bash
./start.sh -h
```

While the monitoring client is started and is collecting metric data, you can use the RESTful APIs provided by the monitoring server to retrieve run-time metrics and corresponding statistics.


## Configuring plug-ins and update intervals
The monitoring client as well as plug-ins are configurable at run-time by a global configuration file named `mf_config.ini`. The configuration is implemented by using an INI file; each section name such as `timings` or `plugins` is enclosed by square brackets. For each section, various parameters can be set. These parameters are custom-defined for each plug-in.
```bash
;PHANTOM Monitoring Client Configuration

[generic]
server = http://localhost:3033/v1
...

[plugins]
mf_plugin_Board_power = on
mf_plugin_CPU_perf = off
...

[timings]
default               = 1000000000ns
update_configuration  = 360s
mf_plugin_Board_power = 1000000000ns
...

[mf_plugin_Board_power]
ACME_BOARD_NAME = baylibre-acme.local
device0:current = on
device0:vshunt = off
device0:vbus = off
device0:power = on
...

```

Several parameters such as the `timing` of the plug-ins or the `server` where the server is running can be configured through this configuration file. The file is called `mf_config.ini` and is located at `dist/mf_config.ini`.


## Acknowledgment
This project is realized through [EXCESS][excess] and [PHANTOM][phantom]. EXCESS is funded by the EU 7th Framework Programme (FP7/2013-2016) under grant agreement number 611183. The PHANTOM project receives funding under the European Union's Horizon 2020 Research and Innovation Programme under grant agreement number 688146.


## Contributing
Find a bug? Have a feature request?
Please [create](https://github.com/excess-project/monitoring-agent/website/issues) an issue.


## Main Contributors
**Fangli Pi, HLRS**
+ [github/hpcfapix](https://github.com/hpcfapix)


## Release History

| Date        | Version | Comment          |
| ----------- | ------- | ---------------- |
| 2017-04-04  | 1.0     | First prototype  |


## License
Copyright (C) 2014,2015 University of Stuttgart

[Apache License v2](LICENSE).


[server]: https://github.com/hpcfapix/phantom_monitoring_server
[excess]: http://www.excess-project.eu
[phantom]: http://www.phantom-project.org