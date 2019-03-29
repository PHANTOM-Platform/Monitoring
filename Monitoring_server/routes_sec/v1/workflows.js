var express = require('express');
var async = require('async');
var dateFormat = require('dateformat');
var router = express.Router();

var middleware = require('./token-middleware');

/**
* @api {get} /workflows 1. Get a list of all available workflows
* @apiVersion 1.0.0
* @apiName GetWorkflows
* @apiGroup Workflows
*
* @apiSuccess {Object} workflowID       Identifier of the workflow
* @apiSuccess {String} workflowID.href  Resource location of the given workflow
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/workflows
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*       "ms2": {
*         "href": "http://localhost:3033/v1/phantom_mf/workflows/ms2"
*       },
*       "infrastructure": {
*         "href": "http://localhost:3033/v1/phantom_mf/workflows/infrastructure"
*       }
*     }
*
* @apiError WorkflowsNotAvailable No workflows found.
*
* @apiErrorExample Error-Response:
*     HTTP/1.1 404 Not Found
*     {
*       "error": "No workflows found."
*     }
*/
	const contentType_text_plain = 'text/plain';

router.get('/', middleware.ensureAuthenticated, function(req, res, next) {
//     var client = req.app.get('elastic'),
//         size = 1000,
//         json = {};
	var elasticsearch = require('elasticsearch');
	var client = new elasticsearch.Client({
		host: "localhost:9400",
		log: 'error'
	});
	client.count({
		index: 'mf',
		type: 'workflows',
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
			res.end("No workflows found.");
			return;
		}

		client.search({
			index: 'mf',
			type: 'workflows',
			size: size
		}, function(error, response) {
			if (error) {
				res.writeHead(500, { 'Content-Type': contentType_text_plain });
				res.end(" "+next(error));
				return; 
			}
			if (response.hits !== undefined) {
				var results = response.hits.hits;
				json = get_details(results);
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
		response = {};
	keys.forEach(function(key) {
		var source = results[key]._source,
			item = JSON.parse(JSON.stringify(source));
//         if (is_defined(source.tasks)) {
//             item.tasks = [];
//             for (var i in source.tasks) {
//                 item.tasks.push(source.tasks[i].name);
//             }
//         }
		response[results[key]._id] = item;
	});
	return response;
}

/**
* @api {get} /workflows/:workflowID 2. Get information about a specific workflow
* @apiVersion 1.0.0
* @apiName GetWorkflow
* @apiGroup Workflows
*
* @apiParam {String} workflowID     Identifier of a workflow
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/workflows/ms2
*
* @apiSuccess (body) {String} application     Identifier of the workflow
* @apiSuccess (body) {String} author          Author name if provided while registering a new workflow
* @apiSuccess (body) {String} optimization    Optimization criterium: time, energy, balanced
* @apiSuccess (body) {Array}  tasks           List of all tasks, of which the workflow is composed
* @apiSuccess (body) {String} tasks.name      Identifier of the task
* @apiSuccess (body) {String} tasks.exec      Executable for the task
* @apiSuccess (body) {String} tasks.cores_nr  Range of CPU cores used for executing the task on
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*       "application": "ms2",
*       "author": "Random Guy",
*       "optimization": "Time",
*       "tasks": [
*         {
*           "name": "T1",
*           "exec": "/home/ubuntu/ms2/t1.sh",
*           "cores_nr": "1-2"
*         },
*         {
*           "name": "T2.1",
*           "exec": "/home/ubuntu/ms2/t21.sh",
*           "previous": "T1",
*           "cores_nr": "1-2"
*          }
*       ]
*     }
*
* @apiError WorkflowNotAvailable Given ID does not refer to a workflow.
*
* @apiErrorExample Error-Response:
*     HTTP/1.1 404 Not Found
*     {
*       "error": "Workflow with the ID '" + workflowID + "' not found."
*     }
*/
router.get('/:id', middleware.ensureAuthenticated, function(req, res, next) {
	var id = req.params.id.toLowerCase(),
		client = req.app.get('elastic'),
		json = {};

	client.get({
		index: 'mf',
		type: 'workflows',
		id: id
	}, function(error, response) {
		if (response.found) {
			res.writeHead(200, {"Content-Type": "application/json"});
			res.end(JSON.stringify(response._source)+"\n");
		} else {
			res.writeHead(400, {"Content-Type": "application/json"});
			res.end("Workflow with the ID '" + id + "' not found.\n");
		}
//         res.json(json);
	});
});

/**
* @api {put} /workflows/:workflowID 3. Register/Update a workflow with a given workflow ID
* @apiVersion 1.0.0
* @apiName PutWorkflowID
* @apiGroup Workflows
*
* @apiParam {String} workflowID     Identifier of the workflow 
*
* @apiExample {curl} Example usage:
*     curl -i http://localhost:3033/v1/phantom_mf/workflows/ms2
*
* @apiParamExample {json} Request-Example:
*     {
*       "application": "ms2",
*       "author": "Random Guy",
*       "optimization": "Time",
*       "tasks": [
*         {
*           "name": "T1",
*           "exec": "/home/ubuntu/ms2/t1.sh",
*           "cores_nr": "1-2"
*         },
*         {
*           "name": "T2.1",
*           "exec": "/home/ubuntu/ms2/t21.sh",
*           "previous": "T1",
*           "cores_nr": "1-2"
*          }
*       ]
*     }
*
* @apiParam (body) {String} application       Identifier of the workflow
* @apiParam (body) {String} author            Author name if provided while registering a new workflow
* @apiParam (body) {String} optimization      Optimization criterium: time, energy, balanced
* @apiParam (body) {Array}  tasks             List of all tasks, of which the workflow is composed
* @apiParam (body) {String} tasks.name        Identifier of the task
* @apiParam (body) {String} [tasks.exec]      Executable for the task
* @apiParam (body) {String} [tasks.cores_nr]  Range of CPU cores used for executing the task on

* @apiSuccess {String} href                   Link to the stored workflow resource
*
* @apiSuccessExample Success-Response:
*     HTTP/1.1 200 OK
*     {
*       "href": "http://localhost:3033/v1/phantom_mf/workflows/ms2",
*     }
*
* @apiError StorageError Given workflow could not be stored.
*
* @apiErrorExample Error-Response:
*     HTTP/1.1 500 Internal Server Error
*     {
*       "error": "Could not create the workflow."
*     }
*/
router.put('/:id', function(req, res, next) {
	var id = req.params.id.toLowerCase(),
		mf_server = req.app.get('mf_server'),
		client = req.app.get('elastic'),
		json = {};

	client.index({
		index: 'mf',
		type: 'workflows',
		id: id,
		body: req.body
	}, function(error, response) {
		if (error !== 'undefined') {
			json.href = mf_server + '/phantom_mf/workflows/' + id;
		} else {
			res.status(500);
			json.error = "Could not create the workflow.";
		}
		res.json(json);
	});
});

module.exports = router;
