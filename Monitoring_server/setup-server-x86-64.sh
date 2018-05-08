#!/bin/bash
#the software is installed in the folder ~/phantom_server
# then the servers (Monitoring, Repository, AppManager) can share them

#instalation of elasticsearch, it will not delete the existing one, because it may contain data
bash setup-es.sh;

#instalation of nodejs
bash setup-nodejs-64.sh;

