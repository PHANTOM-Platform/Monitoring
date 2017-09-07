var express = require('express');
var async = require('async');
var dateFormat = require('dateformat');
var router = express.Router();

router.get('/:deploymentID', function(req, res, next) {
    var deployment = req.params.deploymentID;
	res = deploy_comp(req,res,next,deployment) ;   
});

router.get('/', function(req, res, next) {
    var client = req.app.get('elastic'),
        size = 1000,
        json = {};

    client.count({
        index: 'mf',
        type: 'deploymentcomp'
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
            type: 'deploymentcomp',
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

function  deploy_comp(req, res, next,deployment ){ 
    var client = req.app.get('elastic'),
        size = 1000,
        json = {};  
	var innerQuery = {};
	innerQuery.match_all = {};  	
			
   client.indices.refresh({
        index: 'mf',
        type: 'deploymentcomp'
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        } 
        client.search({
			index: 'mf',
			type: 'deploymentcomp',
			body: {  
				query: { bool: { must: [ { match: { "deployment_id": deployment } } ] } } 
			}
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
}; 

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
		
	if(req.body['deployment_id'] == undefined) {
			res.status(409);
			json.error = "Could not register the deploymentcomp without deployment_id."; 
			res.json(json);	
	}else if(req.body['comp_id'] == undefined) {
			res.status(409);
			json.error = "Could not register the deploymentcomp without comp_id."; 
			res.json(json);		
	}else {
		var deploymentid = req.body['deployment_id'];
		var compid = req.body['comp_id'];
		client.indices.refresh({
			index: 'mf',
			type: 'deploymentcomp'
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			} 
			client.search({
				index: 'mf',
				type: 'deploymentcomp',
				body: {  
					query: { bool: { must: [ { match: { "comp_id":  compid  } } , { match: { "deployment_id" : deploymentid  } } ] } }
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
							type: 'deploymentcomp', 
							body: req.body
						}, function(error, response) {
							if (error !== 'undefined') {
								json.href = mf_server + '/phantom_mf/deploymentcomp/'; 
								res.json(json);
							} else {
								res.status(500);
								json.error = "Could not create the deploymentcomp.";
								res.json(json);
							} 
						});
					} 	
				}else{
					res.status(500);
					json.error = "Could not create the deploymentcomp (123).";
					res.json(json);
				}  
			});
		});  
	}	
}); 


 


router.post('/', function(req, res, next) {
	var mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {}; 
		
	if(req.body['deployment_id'] == undefined) {
			res.status(409);
			json.error = "Could not register the deploymentcomp without deployment_id."; 
			res.json(json);	
	}else if(req.body['comp_id'] == undefined) {
			res.status(409);
			json.error = "Could not register the deploymentcomp without comp_id."; 
			res.json(json);		
	}else {
		var deploymentid = req.body['deployment_id'];
		var compid = req.body['comp_id'];
		client.indices.refresh({
			index: 'mf',
			type: 'deploymentcomp'
		}, function(error, response) {
			if (error) {
				res.status(500);
				return next(error);
			} 
			client.search({
				index: 'mf',
				type: 'deploymentcomp',
				body: {  
					query: { bool: { must: [ { match: { "comp_id":  compid  } } , { match: { "deployment_id" : deploymentid  } } ] } } 
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
							type: 'deploymentcomp', 
							body: req.body
						}, function(error, response) {
							if (error !== 'undefined') {
								json.href = mf_server + '/phantom_mf/deploymentcomp/'; 
								res.json(json);
							} else {
								res.status(500);
								json.error = "Could not create the deploymentcomp.";
								res.json(json);
							} 
						});
					} 	
				}else{
					res.status(500);
					json.error = "Could not create the deploymentcomp (123).";
					res.json(json);
				}  
			});
		});  
	}	
}); 



module.exports = router;
