var express = require('express');
var async = require('async');
var dateFormat = require('dateformat');
var router = express.Router();

var middleware = require('./token-middleware');

router.get('/',middleware.ensureAuthenticated, function(req, res, next) {
	var client = req.app.get('elastic'),
		size = 1000,
		json = {};

	client.count({
		index: 'mf',
		type: 'devices'
	}, function(error, response) {
		if (error) {
			res.status(500);
			return next(error);
		}
		if (response.hits !== undefined) {
			size = response.count;//size = response.hits.total;
		}
		if (size === 0) {
			res.status(404);
			json.error = "No registered Devices.";
			res.json(json);
			return;
		}

		client.search({
			index: 'mf',
			type: 'devices',
			size: size
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			}
			if (response.hits !== undefined) {
				var results = response.hits.hits;
				json = get_details(results);
			}
			res.json(json);
		});
	});
});

function is_defined(variable) {
	return (typeof variable !== 'undefined');
}

function get_details(results) {
	var keys = Object.keys(results),
		response = {};
	keys.forEach(function(key) {
		var source = results[key]._source,
			item = JSON.parse(JSON.stringify(source));
		if (is_defined(source.tasks)) {
			item.tasks = [];
			for (var i in source.tasks) {
				item.tasks.push(source.tasks[i].name);
			}
		}
		response[results[key]._id] = item;
	});
	return response;
}

router.get('/:id',middleware.ensureAuthenticated, function(req, res, next) {
	var id = req.params.id,
		client = req.app.get('elastic'),
		json = {};

	client.get({
		index: 'mf',
		type: 'devices',
		id: id
	}, function(error, response) {
		if (response.found) {
			json = response._source;
		} else {
			json.error = "Device with the ID '" + id + "' not found.";
		}
		res.json(json);
	});
});

//curl -H "Content-Type: application/json" -XPUT localhost:3033/v1/phantom_mf/devices -d '{
//"application":"hpc_simulation","author":"Random Guy","optimization":"Time","tasks":[{"name":"ventilation","exec":"myapp","cores_nr": "1"}]}'

//curl -H "Content-Type: application/json" -XPUT localhost:3033/v1/phantom_mf/devices -d '{   
//"name" : "jojo" , "type" : "intel_x86-64", "total_cores" : 4, "local_timestamp" : "2017-06-01T07:51:06.325", "CPU_usage_rate" : 0.62, "RAM_usage_rate" : 26.351, 
//"swap_usage_rate" : 0.0, "net_throughput" : 284.995, "io_throughput" : 229.593, "server_timestamp" : "2017-06-01T08:51:23.223" }'

router.post('/', function(req, res, next) {
	var mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {}; 
	if(req.body['name'] != undefined) {
		var name = req.body['name'];
		client.indices.refresh({
			index: 'mf',
			type: 'devices'
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			} 
			client.search({
				index: 'mf',
				type: 'devices',
				body: {  
					query: { bool: { must: [ { match: { "name": name } } ] } }
				}
			}, function(error, response) {
				if (error) {
					res.status(500);
					return next(error);
				}  
				//curl -s -XGET localhost:9400/mf/devices/_search -d '{query: { bool: { must: [ { match: { "name": "odroid" } } ] } }  }'
				//{"took":13,"timed_out":false,"_shards":{"total":5,"successful":5,"failed":0},
				//"hits":{"total":1,"max_score":2.7047482,
				//"hits":[{"_index":"mf","_type":"devices","_id":"AV29gRQ2GvNZHKBCDpwD","_score":2.7047482,"_source": {
				//"name" : "odroid", ...,  "server_timestamp" : "2017-06-01T08:51:23.223" 
				//if (response.count !== undefined) {
				if (response.hits !== undefined) {					
					size = response.hits.total;
					var results = response.hits.hits;
					//json = get_details(results); 
					if( size !== 0 ){
						res.status(409); 
						//409 - Conflict
						//Indica que la solicitud no pudo ser procesada debido a un conflicto con el estado actual del recurso que esta identifica.
						//Indicates that the request could not be processed because of a conflict with the current state of the resource it identifies.
						json.error = "Conflict with existing device with the same name, which ID is " + results[0]._id;  
						res.json(json);
					}else{  
						client = req.app.get('elastic');  
						client.index({
							index: 'mf',
							type: 'devices', 
							body: req.body
						}, function(error, response) {
							if (error !== 'undefined') {
								json.href = mf_server + '/phantom_mf/devices/'; 
								res.json(json);
							} else {
								res.status(500);
								json.error = "Could not create the device.";
								res.json(json);
							} 
						});
					} 	
				}else{
					res.status(500);
					json.error = "Could not create the device (123).";
					res.json(json);
				}  
			});
		}); 
	}else{		
			res.status(409);
			json.error = "Could not register the device without name."; 
			res.json(json);
	}	
}); 

router.put('/', function(req, res, next) {
	var mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {}; 
	if(req.body['name'] != undefined) {
		var name = req.body['name'];
		client.indices.refresh({
			index: 'mf',
			type: 'devices'
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			} 
			client.search({
				index: 'mf',
				type: 'devices',
				body: {
					query: { bool: { must: [ { match: { "name": name } } ] } }
				}
			}, function(error, response) {
				if (error) {
					res.status(500);
					return next(error);
				}  
				//curl -s -XGET localhost:9400/mf/devices/_search -d '{query: { bool: { must: [ { match: { "name": "odroid" } } ] } }  }'
				//{"took":13,"timed_out":false,"_shards":{"total":5,"successful":5,"failed":0},
				//"hits":{"total":1,"max_score":2.7047482,
				//"hits":[{"_index":"mf","_type":"devices","_id":"AV29gRQ2GvNZHKBCDpwD","_score":2.7047482,"_source": {
				//"name" : "odroid", ...,  "server_timestamp" : "2017-06-01T08:51:23.223" 
				//if (response.count !== undefined) {
				if (response.hits !== undefined) {					
					size = response.hits.total;
					var results = response.hits.hits;
					//json = get_details(results); 
					if( size !== 0 ){
						res.status(409); 
						//409 - Conflict
						//Indica que la solicitud no pudo ser procesada debido a un conflicto con el estado actual del recurso que esta identifica.
						//Indicates that the request could not be processed because of a conflict with the current state of the resource it identifies.
						json.error = "Conflict with existing device with the same name, which ID is " + results[0]._id;  
						res.json(json);
					}else{  
						client = req.app.get('elastic');  
						client.index({
							index: 'mf',
							type: 'devices', 
							body: req.body
						}, function(error, response) {
							if (error !== 'undefined') {
								json.href = mf_server + '/phantom_mf/devices/'; 
								res.json(json);
							} else {
								res.status(500);
								json.error = "Could not create the device.";
								res.json(json);
							} 
						});
					} 	
				}else{
					res.status(500);
					json.error = "Could not create the device (123).";
					res.json(json);
				}
			});
		}); 
	}else{
			res.status(409);
			json.error = "Could not register the device without name."; 
			res.json(json);
	}	
}); 


function insert_new_device(req) { 
	var mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {}; 
	client.index({
		index: 'mf',
		type: 'devices',
		body: req.body
	}, function(error, response) {
		if (error !== 'undefined') {
			json.href = mf_server + '/phantom_mf/devices/'  ;
		} else {
			res.status(500);
			json.error = "Could not create the device.";
		} 
	});
	return json;
};

module.exports = router;
