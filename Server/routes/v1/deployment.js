var express = require('express');
var async = require('async');
var dateFormat = require('dateformat');
var router = express.Router();

router.get('/', function(req, res, next) {
    var client = req.app.get('elastic'),
        size = 1000,
        json = {};

    client.count({
        index: 'mf',
        type: 'deployment'
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
            type: 'deployment',
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

router.put('/', function(req, res, next) {
	var mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {}; 
	if(req.body['deployment_name'] != undefined) {
		var name = req.body['deployment_name'];
		client.indices.refresh({
			index: 'mf',
			type: 'deployment'
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			} 
			client.search({
				index: 'mf',
				type: 'deployment',
				body: {  
					query: { bool: { must: [ { match: { "deployment_name": name } } ] } }
				}
			}, function(error, response) {
				if (error) {
					res.status(500);
					return next(error);
				}   
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
							type: 'deployment', 
							body: req.body
						}, function(error, response) {
							if (error !== 'undefined') {
								json.href = mf_server + '/phantom_mf/deployment/'; 
								res.json(json);
							} else {
								res.status(500);
								json.error = "Could not create the deployment.";
								res.json(json);
							} 
						});
					} 	
				}else{
					res.status(500);
					json.error = "Could not create the deployment (123).";
					res.json(json);
				}  
			});
		}); 
	}else{		
			res.status(409);
			json.error = "Could not register the deployment without name."; 
			res.json(json);
	}	
}); 
 
router.post('/', function(req, res, next) {
	var mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {}; 
	if(req.body['deployment_name'] != undefined) {
		var name = req.body['deployment_name'];
		client.indices.refresh({
			index: 'mf',
			type: 'deployment'
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			} 
			client.search({
				index: 'mf',
				type: 'deployment',
				body: {  
					query: { bool: { must: [ { match: { "deployment_name": name } } ] } }
				}
			}, function(error, response) {
				if (error) {
					res.status(500);
					return next(error);
				}   
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
							type: 'deployment', 
							body: req.body
						}, function(error, response) {
							if (error !== 'undefined') {
								json.href = mf_server + '/phantom_mf/deployment/'; 
								res.json(json);
							} else {
								res.status(500);
								json.error = "Could not create the deployment.";
								res.json(json);
							} 
						});
					} 	
				}else{
					res.status(500);
					json.error = "Could not create the deployment (123).";
					res.json(json);
				}  
			});
		}); 
	}else{		
			res.status(409);
			json.error = "Could not register the deployment without name."; 
			res.json(json);
	}	
}); 

module.exports = router;
