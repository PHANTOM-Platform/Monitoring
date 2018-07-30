#!/bin/bash
 javac -classpath org.json.jar:apache-httpcomponents-httpcore.jar:. demo.java
 mkdir demo_phantom
 mv *class demo_phantom/
 java  demo_phantom/demo


