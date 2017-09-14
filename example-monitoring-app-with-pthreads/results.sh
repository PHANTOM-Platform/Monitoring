#!/bin/bash
#script to see the stored results

curl -XGET 'localhost:9400/demo_pthread-example/_search?size=2&pretty="true"'
