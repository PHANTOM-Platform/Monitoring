var express = require('express');
var dateFormat = require('dateformat');
var router = express.Router();

var middleware = require('./token-middleware');

/**
* @api {get} /experiments 1. Get a list of all available experiments 
* @apiVersion 1.0.0
* @apiName GetExperiments
* @apiGroup Experiments
*
* @apiParam {String} [workflow] filters results by the given workflow, e.g. 'ms2'
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/experiments
*
* @apiSuccess {Object} executionID       Identifier of an experiment
* @apiSuccess {String} executionID.href  Link to the experiment's details
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*       "AVZ-ll9FGYwmTvCuSnjW": {
*          "href": "http://localhost:3033/v1/phantom_mf/experiments/AVZ-ll9FGYwmTvCuSnjW?workflow=ms2"
*       },
*       "AVZ-kZTjGYwmTvCuSnZV": {
*          "href": "http://localhost:3033/v1/phantom_mf/experiments/AVZ-kZTjGYwmTvCuSnZV?workflow=ms2"
*       },
*       "AVZ-j2hEGYwmTvCuSnVE": {
*          "href": "http://localhost:3033/v1/phantom_mf/experiments/AVZ-j2hEGYwmTvCuSnVE?workflow=ms2"
*       },
*       ...
*     }
*
* @apiError ExperimentsNotAvailable No experiments found.
*
* @apiErrorExample Error-Response:
*     HTTP/1.1 404 Not Found
*     {
*       "error": "No experiments found."
*     }
*/
	const contentType_text_plain = 'text/plain';
router.get('/', middleware.ensureAuthenticated, function(req, res, next) {
	var elasticsearch = require('elasticsearch');
	var client = new elasticsearch.Client({
		host: "localhost:9400",
		log: 'error'
	});
	
	var workflow = req.query.workflow; //.toLowerCase(); 
// 	query = '{ "query": { "match_all": {} } }';
// 	if (typeof workflow !== 'undefined') {
// 		query = '{ "query": { "term": { "_parent": "' + workflow + '" } } }';
// 	} 
	client.count({
		index: 'mf',
		type: 'experiments',
		body:{"query":{"match_all": {} }}
	}, function(error, response) {
		if (error) {
			res.writeHead(500, { 'Content-Type': contentType_text_plain });
			res.end("error: " + error );
			return;
		}
		if (response.count !== undefined) {
			size = response.count;
		}else{
			size = 0;
		}
		if (size === 0) {
			res.writeHead(500, { 'Content-Type': contentType_text_plain });
			res.end("No experiments found.");
			return;
		}
		client.search({
			index: 'mf',
			type: 'experiments',
			fields: '_parent,_source',
// 			body: query,
			size: size,
			sort: '@timestamp:desc'
		}, function(error, response) {
			if (error) {
				res.writeHead(500, { 'Content-Type': contentType_text_plain });
				res.end(" "+next(error));
				return; 
			}
			if (response.hits !== undefined) {
				var results = response.hits.hits;
				json = get_details(results);
			} else {
				json = { "error":"No experiments found" };
			}
			res.writeHead(200, { 'Content-Type': contentType_text_plain });
			res.end( " "+ JSON.stringify( json, null, 4) );
		});
	});
});

function is_defined(variable) {
	return (typeof variable !== 'undefined');
}

function get_details(results) {
	var keys = Object.keys(results),
	item = {},
	response = {};
	keys.forEach(function(key) {
		item = results[key]._source;
		if (typeof item.timestamp !== 'undefined') {
			item.date = item.timestamp.split('-')[0];
			item.date = item.date.replace(/\./g, '-');
			item.started = item.timestamp;
			var secs = item.started.split('-');
			if (secs.length == 2) {
				secs[0] = secs[0].replace(/\./g, '-');
				secs[1] = secs[1].replace(/\./g, ':');
				item.started = secs.join('T');
			}
			delete item.timestamp;
		}
		item.workflow = results[key]._parent;
		response[results[key]._id] = item;
	});
	return response;
}

/**
* @api {get} /experiments/:experimentID 2. Get a registered experiment with given execution ID
* @apiVersion 1.0.0
* @apiName GetExperimentsByID
* @apiGroup Experiments
*
* @apiParam {String} experimentID      Identifier of an experiment
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/experiments/AVZ-ll9FGYwmTvCuSnjW?workflow=ms2
*
* @apiSuccess {String} [application]  Name of the workflow
* @apiSuccess {String} [task]         Name of the task (sub-component of the workflow)
* @apiSuccess {String} [host]         Name of the target platform, where the experiment is conducted
* @apiSuccess {String} [timestamp]    Timestamp when the experiment is registered
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*        "application": "ms2",
*        "task": "t1",
*        "host": "node01",
*        "@timestamp": "2016-08-12T13:49:59"
*     }
*
* @apiError DatabaseError Elasticsearch specific error message.
*/
router.get('/:experimentID', middleware.ensureAuthenticated, function(req, res, next) {
	var elasticsearch = require('elasticsearch');
	var client = new elasticsearch.Client({
		host: "localhost:9400",
		log: 'error'
	}); 	
	var id = req.params.experimentID;
	var workflow = req.query.workflow;//toLowerCase();
	if (typeof workflow == 'undefined') {
		res.writeHead(500, { 'Content-Type': contentType_text_plain });
		res.end("[ERROR] URL parameter 'workflow' is missing\n");
		return;
	} 
	if (typeof id !== 'undefined') {
		query = '{ "query": { "term": { "_id": "' + id + '" } } }'; 
		client.count({
			index: 'mf',
			type: 'experiments',
			body: query
		}, function(error, response) { 
			if (error) {
				res.writeHead(500, { 'Content-Type': contentType_text_plain });
				res.end("error: " + error );
				return;
			}
			if (response.count !== undefined) {
				size = response.count;
			}else{
				size = 0;
			}
			if (size === 0) {
				res.writeHead(500, { 'Content-Type': contentType_text_plain });
				res.end("No experiments found.");
				return;
			} 
			client.get({
				index: 'mf',
				type: 'experiments',
				parent: workflow,
				id: id
			}, function(error, response) {
				if (response.found) {
					json = response._source;
					if (json['@timestamp'] !== 'undefined') {
						delete json.timestamp;
					}
					res.json(json);
				} else {
					res.json(error);
				}
			});
		}); 
	} 
});

/**
* @api {post} /experiments/:workflowID 3. Create a new experiment with given workflow ID
* @apiVersion 1.0.0
* @apiName PostExperiments
* @apiGroup Experiments
*
* @apiParam {String} workflowID          Identifier for the workflow for which the experiment shall be created, e.g. 'ms2'
* @apiParam {String} [application]       Name of the application, same as the workflow ID
* @apiParam {String} [host]              Hostname of the target platform
* @apiParam {String} [author]            Author, like who is registering the experiment
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/experiments/ms2
*
* @apiParamExample {json} Request-Example:
*     {
*       "application": "vector_scal01",
*       "host": "node01",
*       "author": "jmmontanana"
*     }
*
* @apiSuccess {Object} executionID       Identifier of the new registered experiment
* @apiSuccess {String} executionID.href  Link to the new registered experiment
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*       "AVXt3coOz5chEwIt8_Ma": {
*         "href": "http://localhost:3033/v1/phantom_mf/experiments/AVXt3coOz5chEwIt8_Ma?workflow=ms2"
*       }
*     }
* @apiError WorkflowNotFound No workflow as given is found.
*
* @apiErrorExample Error-Response:
*     HTTP/1.1 404 Not Found
*     {
*       "error": "No workflow as '" + workflowID + "' is found."
*     }
*/
router.post('/:workflowID', function(req, res, next) {
	var id = req.params.workflowID.toLowerCase(),
	mf_server = req.app.get('mf_server'),
	client = req.app.get('elastic');

	var body = req.body;
	body['@timestamp'] = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");

	/*check if given workflow exists */
	client.get({
		index: 'mf',
		type: 'workflows',
		id: id
	}, function(err, result) {
		if (err) {
			res.status(500);
			return next(err);
		}
		/*if the workflow can be found */
		if(result !== undefined) {
			client.index({
				index: 'mf',
				type: 'experiments',
				parent: id,
				body: body
			},function(error, response) {
				if (error) {
					res.json(error);
				} else {
					res.send(response._id);
				}
			});
		}
		/*if no such workflow is found */
		else {
			json.error = "No workflow as " + id +" is found.";
			res.json(json);
		}
	}); 
});

module.exports = router;
