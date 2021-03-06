var express = require('express');
var dateFormat = require('dateformat');
var http = require('http');
var async = require('async');
var router = express.Router();

/**
* @api {get} /metrics/:workflowID/:taskID/:experimentID 1. Get all sampled metrics' names and average values with given workflowID, taskID and experimentID
* @apiVersion 1.0.0
* @apiName GetMetrics
* @apiGroup Metrics
*
* @apiParam {String} workflowID        Name of the workflow
* @apiParam {String} taskID            Name of the task
* @apiParam {String} executionID       Identifier of the experiment
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/metrics/ms2/t1/AVNXMXcvGMPeuCn4bMe0
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     [
*       {
*         "device0:current":
*            {
*              "count":55,
*              "min":0,
*              "max":0,
*              "avg":0,
*              "sum":0
*            },
*         "device0:vbus":
*            {
*              "count":55,
*              "min":0,
*              "max":0,
*              "avg":0,
*              "sum":0
*            },
*          "device0:vshunt":
*            {
*              "count":55,
*              "min":0,
*              "max":0,
*              "avg":0,
*              "sum":0
*            },
*          "device0:power":
*            {
*              "count":55,
*              "min":0,
*              "max":0,
*              "avg":0,
*              "sum":0
*            }
*       }
*     ]
*
* @apiError DatabaseError Elasticsearch specific error message.
*/
router.get('/:workflowID/:taskID/:experimentID', function(req, res, next) {
	var client = req.app.get('elastic'),
		workflow = req.params.workflowID,
		task = req.params.taskID,
		experiment = req.params.experimentID,
		size = 1000,
		json = [];

	var data = {},
		metrics = {},
		index = workflow + '_' + task;
	
	client.search({
		index: index,
		type: experiment,
		size: size
	}, function (error, response) {
		if(error) {
			res.status(500);
			return next(error);
		}
		if(response.hits !== undefined) {
			var results = response.hits.hits,
				keys = Object.keys(results),
				items = {};
			/* filter keys like local_timestamp, server_timestamp, host, task, and type */
			keys.forEach(function (key) {
				items = results[key]._source;
				delete items.local_timestamp;
				delete items.server_timestamp;
				delete items.host;
				delete items.task;
				delete items.TaskID;
				delete items.type;
				delete items.component_name;
				delete items.component_start;
				delete items.component_end;
				delete items.component_duration;
				for (var item in items) {
					metrics[item] = item;
				}
			});
		}

		async.each(metrics, function(metric, inner_callback) {
			var myquery=  aggregation_by(metric);
			client.search({
				index: index,
				type: experiment,
				body: myquery
			}, function(error, response) {
				if (error || response == undefined) { 
					inner_callback(null);
				}else{  
					var aggs = response.aggregations;
					data[metric] = aggs[metric + '_Stats']; 
					inner_callback(null);
				}
			});
		}, function() {
			json.push(data);
			res.json(json);// (JSON.stringify(json,null,4)));
		});
	});
});


function lowercase(input_string){
	var result="";
	for (var j = 0; j < input_string.length; j++) {
// 		input_string.replaceAt(j, character.toLowerCase());
		var charCode = input_string.charCodeAt(j);
		if (charCode < 65 || charCode > 90) {
			// NOT an uppercase ASCII character
			// Append the original character
			result += input_string.substr(j, 1);
		} else {
			// Character in the ['A'..'Z'] range
			// Append the lowercase character
			result += String.fromCharCode(charCode + 32);
		}
	}
	return (result);
}


/**
* @api {post} /metrics 3. Send an array of metrics
* @apiVersion 1.0.0
* @apiName PostBulkMetrics
* @apiGroup Metrics
*
* @apiParam (body) {String} WorkflowID      Name of the application
* @apiParam (body) {String} TaskID          Name of the task
* @apiParam (body) {String} ExperimentID    Identifier of the experiment
* @apiParam (body) {String} [type]          Type of the metric, e.g. power, temperature, and so on
* @apiParam (body) {String} [host]          Hostname of the target platform
* @apiParam (body) {String} local_timestamp Local timestamp, when the metric is collected
* @apiParam (body) {String} metric          Name and value of the metric
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/metrics
*
* @apiParamExample {json} Request-Example:
*     [
*       {
*         "WorkflowID": "ms2",
*         "ExperimentID": "AVUWnydqGMPeuCn4l-cj",
*         "TaskID": "t2.1",
*         "local_timestamp": "2016-02-15T12:43:48.749",
*         "type": "power",
*         "host": "node01.excess-project.eu",
*         "GPU1:power": "168.519"
*       }, {
*         "WorkflowID": "ms2",
*         "ExperimentID":"AVNXMXcvGMPeuCn4bMe0",
*         "TaskID": "t2.2",
*         "local_timestamp": "2016-02-15T12:46:48.524",
*         "type": "power",
*         "host": "node01.excess-project.eu",
*         "GPU0:power": "152.427"
*       }
*     ]
*
* @apiSuccess {String} href links to all updated profiled metrics
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     [
*       "http://localhost:3033/v1/phantom_mf/profiles/ms2/t2.1/AVUWnydqGMPeuCn4l-cj",
*       "http://localhost:3033/v1/phantom_mf/profiles/ms2/t2.2/AVNXMXcvGMPeuCn4bMe0"
*     ]
*
* @apiError DatabaseError Elasticsearch specific error message.
*/
router.post('/', function(req, res, next) {
// 	var data = req.body,
// 		mf_server = req.app.get('mf_server'),
// 		client = req.app.get('elastic');
	var bulk_data = [];
	var data = req.body;

	var mf_server = "localhost:9400";
	var elasticsearch = require('elasticsearch');
	var client = new elasticsearch.Client({
		host: "localhost:9400",
		log: 'error'
	});
	
//  	console.log("[metrics] data is "+JSON.stringify(data)+" \n");
	
	const contentType_text_plain = 'text/plain';
// 	console.log("data is "+JSON.stringify(data)+"\n");
// // 			res.end("data is "+JSON.stringify(data)+"\n", 'utf-8');
// 			res.writeHead(600, { 'Content-Type': contentType_text_plain });
// 			res.end("data is " +"\n");
// 			return("data is " +"\n");
	if(data.length>1){
		var tmp = {};
		tmp.index = {};
		for (i = 0; i != data.length; ++i) {
			var action = JSON.parse(JSON.stringify(tmp));
			var index = data[i].WorkflowID;
			if (data[i].TaskID) {
				index = index + '_' + data[i].TaskID;
			} else {
				index = index + '_all';
			}
			index=lowercase(index); //index can not have capital letters !!!
			/*
			if(data[i]['@timestamp'] == undefined) {
			data[i]['@timestamp'] = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
			}*/
			data[i].server_timestamp = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
			if(data[i].local_timestamp == undefined) {
				data[i].local_timestamp = data[i].server_timestamp;
			} else {
				var tmp_value = parseInt(data[i].local_timestamp);
				data[i].local_timestamp = dateFormat(new Date(tmp_value), "yyyy-mm-dd'T'HH:MM:ss.l");
			}
			action.index._index = index;
			action.index._type = data[i].ExperimentID;
			delete data[i].WorkflowID;
			delete data[i].ExperimentID;
			bulk_data.push(action);
			bulk_data.push(data[i]);
		}


		client.bulk({
			body: bulk_data
		},function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			}
			var json = [];
// 			console.log("[metrics] response is "+JSON.stringify(response)+" \n");
			for (var i in response.items) {
				json.push(mf_server + '/phantom_mf/profiles/' +
				response.items[i].create._index.replace('_all', '/all') +
				'/' + response.items[i].create._type);
			}
// 			res.end(JSON.stringify(json));
			res.end("aaa");
		});
	}else{ 
		var index = data[0].WorkflowID;
			if (data[0].TaskID) {
				index = index + '_' + data[0].TaskID;
			} else {
				index = index + '_all';
			}
			/*
			if(data[i]['@timestamp'] == undefined) {
			data[i]['@timestamp'] = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
			}*/
			data[0].server_timestamp = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
			if(data[0].local_timestamp == undefined) {
				data[0].local_timestamp = data[i].server_timestamp;
			} else {
				var tmp_value = parseInt(data[0].local_timestamp);
				data[0].local_timestamp = dateFormat(new Date(tmp_value), "yyyy-mm-dd'T'HH:MM:ss.l");
			}
		
		var mytype=data[0].ExperimentID;
		delete data[0].WorkflowID;
		delete data[0].ExperimentID;
		var bulk_data = data[0];
		client.index({
			index: index,
			type: mytype,
			body: bulk_data
		}, function(error, response) {
			if (error !== 'undefined') { 
				res.status(400);
				res.end("error: "+error +"\n");
			} else {
				res.status(420);
				res.end("error: "+error +"\n");
			}
		});//end query client.index
		res.status(200);
		res.end("success"+"\n");
	}
});

/**
* @api {post} /metrics/:workflowID/:taskID/:experimentID 2. Send a metric with given workflow ID, task ID, and experiment ID
* @apiVersion 1.0.0
* @apiName PostMetric
* @apiGroup Metrics
*
* @apiParam {String} WorkflowID              Name of the application
* @apiParam {String} TaskID                  Name of the task
* @apiParam {String} ExperimentID            Identifier of the experiment
* @apiParam (body) {String} [type]           Type of the metric, e.g. power, temperature, and so on
* @apiParam (body) {String} [host]           Hostname of the target platform
* @apiParam (body) {String} local_timestamp  Local timestamp, when the metric is collected
* @apiParam (body) {String} metric           Name and value of the metric
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/metrics/ms2/t1/AVNXMXcvGMPeuCn4bMe0
*
* @apiParamExample {json} Request-Example:
*     {
*       "type": "power",
*       "host": "node01.excess-project.eu",
*       "local_timestamp": "2016-02-15T12:42:22.000",
*       "GPU0:power": "152.427"
*     }
*
* @apiSuccess {Object} metricID       Identifier of the sent metric
* @apiSuccess {String} metricID.href  Link to the experiment with updated metrics
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*       "AVXt3coOz5chEwIt8_Ma": {
*         "href": "http://localhost:3033/v1/phantom_mf/profiles/ms2/t1/AVNXMXcvGMPeuCn4bMe0"
*       }
*     }
*
* @apiError DatabaseError Elasticsearch specific error message.
*/
router.post('/:workflowID/:taskID/:experimentID', function(req, res, next) {
	var workflowID = req.params.workflowID,
	experimentID = req.params.experimentID,
	taskID = req.params.taskID,
	mf_server = req.app.get('mf_server'),
	client = req.app.get('elastic'),
	index_missing = false;

	var index = workflowID + '_' + taskID;

	async.series([
		function(callback) {
			client.indices.exists({
				index: index
			}, function(error, response) {
				index_missing = !response;
				callback(null, '1=' + index_missing);
			});
		},
		function(callback) {
			var created = false;
			if (index_missing) {
				var headers = {
					'Content-Type': 'application/json',
					'Content-Length': bodyString.length
				};

				var options = {
					host: 'localhost',
					path: '/' + index,
					port: 9400,
					method: 'PUT',
					headers: headers
				};

				var http_request = http.request(options, function(res) {
					res.setEncoding('utf-8');
					res.on('data', function(data) {
						console.log('incoming: ' + data);
					});
					res.on('end', function() {
						callback(null, '2=created');
					});
				});

				http_request.on('error', function(e) {
					callback(null, '2=not_created');
				});

				http_request.write(bodyString);
				http_request.end();
			} else {
				callback(null, '2=exists');
			}
		},
		function(callback) {
			/* work-around for plug-ins sending the old timestamp format */
			if (req.body.Timestamp) {
				req.body['@timestamp'] = req.body.Timestamp;
				delete req.body.Timestamp;
			}
			/*
			if(req.body['@timestamp'] == undefined) {
			req.body['@timestamp'] = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
			}*/
			req.body.server_timestamp = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
			if(req.body.local_timestamp == undefined) {
				req.body.local_timestamp = req.body.server_timestamp;
			}

			/* fix for timestamps having whitespaces: 2016-08-24T10:24:07.  6 */
			if (req.body['@timestamp'] !== undefined) {
				var replaced = req.body['@timestamp'].replace(/ /g, '0');
				req.body['@timestamp'] = replaced;
			}
			client.index({
				index: index,
				type: experimentID,
				body: req.body
			},function(error, response) {
			if (error) {
				res.status(500);
				callback(null, 'id not found');
				return;
			}
			var json = {};
			json[response._id] = {};
			json[response._id].href = mf_server + '/phantom_mf/profiles/' + workflowID;
			if (typeof taskID !== 'undefined') {
				json[response._id].href += '/' + taskID;
			}
			json[response._id].href += '/' + experimentID;
			res.json(json);
			callback(null, '3');
		});
	}
	],
	function(err, results){
	});
});

var bodyString =
'{' +
'"dynamic_templates": [' +
	'{' +
		'"string_fields": {' +
			'"mapping": {' +
			'"index": "analyzed",' +
			'"omit_norms": true,' +
			'"type": "string",' +
			'"fields": {' +
				'"raw": {' +
					'"index": "not_analyzed",' +
					'"ignore_above": 256,' +
					'"type": "string"' +
				'}' +
			'}' +
			'},' +
			'"match": "*",' +
			'"match_mapping_type": "string"' +
		'}' +
	'}' +
'],' +
'"_all": {' +
	'"enabled": true' +
'},' +
'"properties": {' +
	'"@timestamp": {' +
		'"enabled" : true,' +
		'"type":"date",' +
		'"format": "date_hour_minute_second_millis",' +
		'"store": true,' +
		'"path": "@timestamp"' +
	'},' +
	'"host": {' +
		'"type": "string",' +
		'"norms": {' +
			'"enabled": false' +
		'},' +
		'"fields": {' +
			'"raw": {' +
			'"type": "string",' +
			'"index": "not_analyzed",' +
			'"ignore_above": 256' +
			'}' +
		'}' +
	'},' +
	'"name": {' +
		'"type": "string",' +
		'"norms": {' +
			'"enabled": false' +
		'},' +
		'"fields": {' +
			'"raw": {' +
			'"type": "string",' +
			'"index": "not_analyzed",' +
			'"ignore_above": 256' +
			'}' +
		'}' +
	'},' +
	'"value": {' +
		'"type": "long",' +
		'"norms": {' +
			'"enabled": false' +
		'},' +
		'"fields": {' +
			'"raw": {' +
			'"type": "long",' +
			'"index": "not_analyzed",' +
			'"ignore_above": 256' +
			'}' +
		'}' +
	'}' +
'}' +
'}';

function aggregation_by(field_name) {
	return '{' +
		'"aggs": {' +
			'"' + field_name + '_Stats" : {' +
				'"stats" : {' +
					'"field" : "' + field_name + '"' +
				'}' +
			'},' +
			'"Minimum_' + field_name + '": {' +
				'"top_hits": {' +
					'"size": 1,' +
					'"sort": [' +
						'{' +
							'"' + field_name + '": {' +
								'"order": "asc"' +
							'}' +
						'}' +
					']' +
				'}' +
			'},' +
			'"Maximum_' + field_name + '": {' +
				'"top_hits": {' +
					'"size": 1,' +
					'"sort": [' +
						'{' +
							'"' + field_name + '": {' +
								'"order": "desc"' +
							'}' +
						'}' +
					']' +
				'}' +
			'}' +
		'}' +
	'}';
}

function isEmpty(obj) {
	for(var key in obj) {
		if(obj.hasOwnProperty(key))
		return false;
	}
	return true;
}

module.exports = router;
