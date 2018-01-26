#!/bin/bash
#  Copyright (C) 2015, 2016 University of Stuttgart
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

#Note: the instalation process will require to use more than 800MB of harddisk space. The required space after the instalation is about 200MB.
#Note: the instalation needs to be done in a linux system because some libs need to create symbolic links (avoid FAT and NTFS filesystems).
# ============================================================================ #
# DEFAULT PATHS                                                                #
# ============================================================================ #
	ROOT=`pwd`;
	BINARY_FOLDER="bin";
	INSTALL_PATH_HWLOC=`pwd`/${BINARY_FOLDER}/hwloc;
	INSTALL_PATH_PAPI=`pwd`/${BINARY_FOLDER}/papi;
	INSTALL_PATH_APR=`pwd`/${BINARY_FOLDER}/apr;
	INSTALL_PATH_APU=${INSTALL_PATH_APR};
	INSTALL_PATH_CURL=`pwd`/${BINARY_FOLDER}/curl;
	INSTALL_PATH_NVIDIA=`pwd`/${BINARY_FOLDER}/nvidia;
	INSTALL_PATH_M4=`pwd`/${BINARY_FOLDER}/m4;
	INSTALL_PATH_BISON=`pwd`/${BINARY_FOLDER}/bison;
	INSTALL_PATH_FLEX=`pwd`/${BINARY_FOLDER}/flex;
	INSTALL_PATH_SENSORS=`pwd`/${BINARY_FOLDER}/sensors;
	INSTALL_PATH_EXCESS_QUEUE=`pwd`/ext/queue;
	INSTALL_PATH_LIBIIO=`pwd`/${BINARY_FOLDER}/libiio;
# ============================================================================ #
# VERSIONS OF REQUIRED LIBRARIES                                               #
# ============================================================================ #
	PAPI="papi";
	PAPI_VERSION="5.4.0";
	CURL="curl";
	CURL_VERSION="7.37.0";
	APR="apr";
	APR_VERSION="1.6.3";  #before was 1.5.2
	APR_UTIL="apr-util";
	APR_UTIL_VERSION="1.6.1";  #before was 1.5.4
	EXCESS_QUEUE_VERSION=release/0.1.0;
#Notice: to reduce expose to IPv6 connectivity issues, we force all wget commands to connect via IPv4 only, with the pareameter -4  or --inet4-only
# ============================================================================ #
# DOWNLOAD AND INSTALL HWLOC                                                   #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f hwloc-1.11.2.tar.gz ]; then
		wget --no-check-certificate -4 https://www.open-mpi.org/software/hwloc/v1.11/downloads/hwloc-1.11.2.tar.gz;
	fi;
	if [ ! -f hwloc-1.11.2.tar.gz ]; then
		echo "[ERROR] File not found: " hwloc-1.11.2.tar.gz;
		exit 1;
	fi;
	tar xf hwloc-1.11.2.tar.gz;

	cd hwloc-1.11.2;
	./configure --prefix=${INSTALL_PATH_HWLOC}
	make
	make install
# ============================================================================ #
# DOWNLOAD AND INSTALL PAPI-C                                                  #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f ${PAPI}-${PAPI_VERSION}.tar.gz ]; then
		wget -4 http://icl.cs.utk.edu/projects/papi/downloads/${PAPI}-${PAPI_VERSION}.tar.gz;
	fi;
	if [ ! -f ${PAPI}-${PAPI_VERSION}.tar.gz ]; then
		echo "[ERROR] File not found: " ${PAPI}-${PAPI_VERSION}.tar.gz;
		exit 1;
	fi
	tar zxvf ${PAPI}-${PAPI_VERSION}.tar.gz;
	cd ${PAPI}-${PAPI_VERSION}/src;
	./configure --prefix=${INSTALL_PATH_PAPI} --with-components="rapl coretemp infiniband"
	make
	sudo make install all
# ============================================================================ #
# DOWNLOAD AND INSTALL CURL                                                    #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f ${CURL}-${CURL_VERSION}.tar.gz ]; then
		wget --no-check-certificate -4 http://curl.haxx.se/download/${CURL}-${CURL_VERSION}.tar.gz;
	fi;
	if [ ! -f ${CURL}-${CURL_VERSION}.tar.gz ]; then
		echo "[ERROR] File not found: " ${CURL}-${CURL_VERSION}.tar.gz;
		exit 1;
	fi
	tar zxvf ${CURL}-${CURL_VERSION}.tar.gz;
	cd ${CURL}-${CURL_VERSION};
	./configure --prefix=${INSTALL_PATH_CURL}
	make
	make install
	make install all
# ============================================================================ #
# DOWNLOAD AND INSTALL APACHE APR                                              #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f ${APR}-${APR_VERSION}.tar.gz ]; then
		wget --no-check-certificate -4 http://www.eu.apache.org/dist/apr/${APR}-${APR_VERSION}.tar.gz;
	fi;
	if [ ! -f ${APR}-${APR_VERSION}.tar.gz ]; then
		echo "[ERROR] File not found: " ${APR}-${APR_VERSION}.tar.gz ;
		exit 1;
	fi;
	tar zxvf ${APR}-${APR_VERSION}.tar.gz ;
	cd ${APR}-${APR_VERSION} ;
	./configure --prefix=${INSTALL_PATH_APR}
	make
	make install
	make install all
# ============================================================================ #
# DOWNLOAD AND INSTALL APACHE APR UTILITIES                                    #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f ${APR_UTIL}-${APR_UTIL_VERSION}.tar.gz ]; then
		wget -4 http://www.eu.apache.org/dist/apr/${APR_UTIL}-${APR_UTIL_VERSION}.tar.gz;
	fi;
	if [ ! -f ${APR_UTIL}-${APR_UTIL_VERSION}.tar.gz ]; then
		echo "[ERROR] File not found: " ${APR_UTIL}-${APR_UTIL_VERSION}.tar.gz;
		exit 1;
	fi
	tar zxvf ${APR_UTIL}-${APR_UTIL_VERSION}.tar.gz;
	cd ${APR_UTIL}-${APR_UTIL_VERSION};
	./configure --prefix=${INSTALL_PATH_APU} --with-apr=${INSTALL_PATH_APR};
	make
	make install
	make install all
# ============================================================================ #
# DOWNLOAD AND INSTALL NVIDIA GDK                                              #
# ============================================================================ #
	cd $ROOT;
	mkdir -p nvidia_gdk_download;
	cd nvidia_gdk_download;
	NVIDIA_BASE_URL="http://developer.download.nvidia.com";
	NVIDIA_GDK="cuda_340_29_gdk_linux_32.run";
	if [ ! -f ${NVIDIA_GDK} ]; then
		wget -4 ${NVIDIA_BASE_URL}/compute/cuda/6_5/rel/installers/${NVIDIA_GDK};
	fi;
	if [ ! -f ${NVIDIA_GDK} ]; then
		echo "[ERROR] File not found: " ${NVIDIA_GDK};
		exit 1;
	fi
	chmod +x ${NVIDIA_GDK};
	./${NVIDIA_GDK} --silent --installdir=${INSTALL_PATH_NVIDIA}
# ============================================================================ #
# DOWNLOAD AND INSTALL SENSORS LIB                                             #
# https://fossies.org/linux/misc/lm_sensors-3.4.0.tar.gz/lm_sensors-3.4.0/lib/libsensors.3
# ============================================================================ #
#
# DEPENDENCIES: bison and flex
#
	cd $ROOT;
	if [ ! -f m4-1.4.17.tar.gz ]; then
		wget -4 https://ftp.gnu.org/gnu/m4/m4-1.4.17.tar.gz;
	fi;
	if [ ! -f m4-1.4.17.tar.gz]; then
		echo "[ERROR] File not found: " m4-1.4.17.tar.gz;
		exit 1;
	fi
	tar zxvf m4-1.4.17.tar.gz
	cd m4-1.4.17;
	./configure --prefix=${INSTALL_PATH_M4}
	make
	make install
	export PATH=${PATH}:${INSTALL_PATH_M4}/bin
# ============================================================================ #
# DOWNLOAD AND INSTALL bison                                            #
# ============================================================================ #
	cd $ROOT
	if [ ! -f bison-3.0.2.tar.gz ]; then
		wget -4 http://ftp.gnu.org/gnu/bison/bison-3.0.2.tar.gz;
	fi;
	if [ ! -f bison-3.0.2.tar.gz ]; then
		echo "[ERROR] File not found: " bison-3.0.2.tar.gz;
		exit 1;
	fi;
	tar zxvf bison-3.0.2.tar.gz;
	cd bison-3.0.2;
	./configure --prefix=${INSTALL_PATH_BISON}
	make
	make install
# ============================================================================ #
# DOWNLOAD AND INSTALL flex                                             #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f flex-2.6.0.tar.gz ]; then
		wget -4 http://prdownloads.sourceforge.net/flex/flex-2.6.0.tar.gz
	fi;
	if [ ! -f flex-2.6.0.tar.gz ]; then
		echo "[ERROR] File not found: " flex-2.6.0.tar.gz;
		exit 1;
	fi;
	tar zxvf flex-2.6.0.tar.gz;
	cd flex-2.6.0;
	./configure --prefix=${INSTALL_PATH_FLEX};
	make
	make install

	export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:${INSTALL_PATH_FLEX}/lib:${INSTALL_PATH_BISON}/lib
	export PATH=${PATH}:${INSTALL_PATH_BISON}/bin:${INSTALL_PATH_FLEX}/bin
# ============================================================================ #
# DOWNLOAD AND INSTALL lm_sensors                                            #
# ============================================================================ #
	cd $ROOT;
	if [ ! -f lm_sensors-3.4.0.tar.gz ]; then
		wget -4 https://fossies.org/linux/misc/lm_sensors-3.4.0.tar.gz
	fi;
	if [ ! -f lm_sensors-3.4.0.tar.gz ]; then
		echo "[ERROR] File not found: " lm_sensors-3.4.0.tar.gz;
		exit 1;
	fi;
	tar zxvf lm_sensors-3.4.0.tar.gz;
	cd lm_sensors-3.4.0
	make PREFIX=${INSTALL_PATH_SENSORS} all
	make PREFIX=${INSTALL_PATH_SENSORS} install
# ============================================================================ #
# DOWNLOAD AND INSTALL LIBIIO                                                  #
# https://wiki.analog.com/resources/tools-software/linux-software/libiio
# ============================================================================ #
#
# DEPENDENCIES: libxml2 libxml2-dev bison flex libcdk5-dev libavahi-client-dev cmake
#
	cd $ROOT;
	if [ ! -f libiio.tgz ]; then
		git clone https://github.com/analogdevicesinc/libiio.git
	else
		tar -xzf libiio.tgz;
	fi;
	cd libiio;
	mkdir -p ${INSTALL_PATH_LIBIIO};
	cmake -DCMAKE_INSTALL_PREFIX=${INSTALL_PATH_LIBIIO} ./
	make all
	sudo make install
	find ./ -name "libiio.so*" -exec sudo mv {} ${INSTALL_PATH_LIBIIO}/lib/ \;
# ============================================================================ #
# DOWNLOAD AND INSTALL EXCESS QUEUE LIBS                                                                                        #
# https://github.com/excess-project/data-structures-library.git
# ============================================================================ #
	cd $INSTALL_PATH_EXCESS_QUEUE;
	rm -rf data-structures-library;
	if [ ! -f data-structures-library.tgz ]; then
		git clone https://github.com/excess-project/data-structures-library.git
	else
		tar -xzf data-structures-library.tgz;
	fi;
	cd data-structures-library;
	git checkout $EXCESS_QUEUE_VERSION
# ============================================================================ #
# CLEANING UP                                                                  #
# ============================================================================ #
	cd $ROOT
	# Consider to keep the files in case you need to reinstall without internet access.
	#if [ "1" == "0" ]; then
		rm -rf *.tar.gz *.tgz ;
		rm -rf nvidia_gdk_download ;
		rm -rf ${NVIDIA_GDK} ;
	#fi;
	rm -rf hwloc-1.11.2 ;
	rm -rf ${PAPI}-${PAPI_VERSION} ;
	rm -rf ${LIKWID}-${LIKWID_VERSION} ;
	rm -rf ${APR}-${APR_VERSION} ;
	rm -rf ${APR_UTIL}-${APR_UTIL_VERSION} ;
	rm -rf ${CURL}-${CURL_VERSION} ;
	rm -rf m4-1.4.17 ;
	rm -rf bison-3.0.2 ;
	rm -rf flex-2.6.0 ;
	rm -rf lm_sensors-3.4.0 ;
	rm -rf libiio ;
# ============================================================================ #
# DONE
# ============================================================================ #
echo "ALL DEPENDENCIES WERE INSTALLED SUCCESSFULLY!";

