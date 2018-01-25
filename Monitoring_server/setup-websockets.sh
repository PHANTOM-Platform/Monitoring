#!/bin/bash

echo -e "#####################################################################################";
echo -e "##                                   ATENTION                                      ##";
echo -e "#####################################################################################";
echo -e "\n";
echo -e "The websocket plugin needs a security permission for listen and resolve on port 9800,";
echo -e " be added to the grant permissions on the java police file"; 
echo -e "(usually the list of default permissions are at the end of that file)\n\n" 
echo -e "The format of the permission is: "

echo -e "    // default permissions granted to all domains";
echo -e "    grant {"; 
echo -e "        ..."; 
echo -e "        ..."; 
echo -e "        permission java.net.SocketPermission \"localhost:9800\", \"listen,resolve\""; 
echo -e "    };\n";

echo -e " The location of the java policy file depends on you version of java, it may be like:";
echo -e "     /usr/lib/jvm/java-8-oracle/jre/lib/security/java.policy";
echo -e " or:"
echo -e "     /usr/lib/jvm/java-8-openjdk-amd64/jre/lib/security/java.policy\n"; 

read -p "Press enter to continue"
#read -n 1 -s -r -p "Press any key to continue"
echo -e "\n";

if [ -d Monitoring-master ]; then cd Monitoring-master; fi; 
if [ -d Server ]; then cd Server; fi; 
cd dist
cd elasticsearch
cd bin

echo -e "Removing previous instalation";
sudo ./plugin remove es-changes-feed-plugin

echo -e "Instaling the plugin ...";
sudo ./plugin install -b https://github.com/ForgeRock/es-change-feed-plugin/releases/download/v2.4.0/es-changes-feed-plugin.zip


