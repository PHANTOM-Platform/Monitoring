var express = require('express');
var async = require('async');
var dateFormat = require('dateformat');
var router = express.Router();
 
router.get('/:indexID/:experimentID', function(req, res, next) {  
	var index = req.params.indexID;
	var experiment = req.params.experimentID;
	res = agg_summary(req,res,next, index, experiment) ;
});

router.get('/', function(req, res, next) {  
   res = agg_summary(req,res,next,'bench_t1', 'AV29MeMni9gmp2734xfP') ;
});

router.get('/:indexID', function(req, res, next) {
    var index = req.params.indexID;
	res = agg_summary_all(req,res,next,index) ;   
});


function agg_summary_all(req, res, next,index ){ 
    var client = req.app.get('elastic'),
        size = 1000,
        json = {}; 
	//var index = 'bench_t1'; 
	var innerQuery = {};
	innerQuery.match_all = {};
   //         res.status(body);
   //         json.error = body;
   //         res.json(json);
   //         return;
			
			
 //   client.get({
 //       index: index 
 //   }, function(error, response) {
 //       if (response.found) {
 //           json = response._source;
 //       } else {
 //           json.error = "Experiment-index with the ID '" + index + "' not found.";
	//		res.json(json);
	//		return;
 //       } 
 //   });    	
			
   client.indices.refresh({
        index: index 
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        }
        //if (response.hits !== undefined) {
        //    size = response.count;//size = response.hits.total;
        //}
        //if (size === 0) {
        //    res.status(404);
        //    json.error = "No registered Devices.";
        //    res.json(json);
        //    return;
        //}     
        client.search({
            index: index,   
			body: {
				//query: innerQuery,  
				//query : body  //performs query":{body},
				query: { "match_all": {} },
				_source: ["host", "TaskID", "platform_id"],	
				aggs: {
					"experiments": { 
						terms: { field: "_type"}, 
						aggs: {  
							"platform_id": {
								top_hits: {
									_source: { includes: [ "platform_id" ] },
									size : 1
								}
							},
							"RAM":{ stats:{ field: "RAM_usage_rate" }},			
							"CPU":{ stats:{ field: "CPU_usage_rate" }},
							"net_throughput":{ stats:{ field: "net_throughput" }},
							"STARTTIME": { min:{ field: "local_timestamp" }},
							"ENDTIME": { max:{ field: "local_timestamp" }},
							"totaltime": {
								bucket_script: {
									buckets_path: {
										"repend": "ENDTIME",
										"repstart": "STARTTIME"
									},
									script: "repend - repstart*1 "
								}
							}
						} 
					}
				}
			}
        }, function(error, response) {
            if (error) {
                res.status(500);
                return next(error);
            }
            if (response.hits !== undefined) {
                //var results = response.hits.hits;//json = get_details(results);
				var resultsaggs = response.aggregations;  
            }
            res.json(resultsaggs );
        });
    });
}; 


function agg_summary(req, res, next,index, experiment){ 
    var client = req.app.get('elastic'),
        size = 1000,
        json = {}; 
	//var index = 'bench_t1'; 
	var innerQuery = {};
	innerQuery.match_all = {};
   //         res.status(body);
   //         json.error = body;
   //         res.json(json);
   //         return;
			
   client.indices.refresh({
        index: index 
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        }
        //if (response.hits !== undefined) {
        //    size = response.count;//size = response.hits.total;
        //}
        //if (size === 0) {
        //    res.status(404);
        //    json.error = "No registered Devices.";
        //    res.json(json);
        //    return;
        //}     
        client.search({
            index: index, 
			type: experiment, 
			body: {
				//query: innerQuery,  
				//query : body  //performs query":{body},
				query: { "match_all": {} },
				_source: ["host", "TaskID", "platform_id"],	
				aggs: {
					"experiments": { 
						terms: { field: "_type"}, 
						aggs: {  
							"platform_id": {
								top_hits: {
									_source: { includes: [ "platform_id" ] },
									size : 1
								}
							},
							"RAM":{ stats:{ field: "RAM_usage_rate" }},			
							"CPU":{ stats:{ field: "CPU_usage_rate" }},
							"net_throughput":{ stats:{ field: "net_throughput" }},
							"STARTTIME": { min:{ field: "local_timestamp" }},
							"ENDTIME": { max:{ field: "local_timestamp" }},
							"totaltime": {
								bucket_script: {
									buckets_path: {
										"repend": "ENDTIME",
										"repstart": "STARTTIME"
									},
									script: "repend - repstart*1 "
								}
							}
						} 
					}
				}
			}
        }, function(error, response) {
            if (error) {
                res.status(500);
                return next(error);
            }
            if (response.hits !== undefined) {
                //var results = response.hits.hits;//json = get_details(results);
				var resultsaggs = response.aggregations;  
            }
            res.json(resultsaggs );
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



module.exports = router;
