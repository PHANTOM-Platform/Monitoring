var express = require('express');
var async = require('async');
var router = express.Router();

/**
 * @api {get} /runtime/:workflowID/:experimentID 1. Get runtime information with given workflow ID and experiment ID
 * @apiVersion 1.0.0
 * @apiName GetRuntimeByExperiment
 * @apiGroup Runtime
 *
 * @apiParam {String} workflowID    Identifier of a workflow
 * @apiParam {String} experimentID  Identifier of an experiment
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/runtime/ms2/AVZ-5cqVGYwmTvCuSqZC
 *
 * @apiSuccess {String} workflow           Identifier of the workflow
 * @apiSuccess {String} start              Start local timestamp of the entire experiment
 * @apiSuccess {String} end                End local timestamp of the entire experiment
 * @apiSuccess {String} total_runtime      Duration of the entire experiment in seconds
 * @apiSuccess {Array}  tasks              Array of task-specific runtime information
 * @apiSuccess {String} tasks.task         Identifier of the task
 * @apiSuccess {Object} tasks.data         Object holding runtime data of the task
 * @apiSuccess {String} tasks.data.start   Start local timestamp of the task
 * @apiSuccess {String} tasks.data.end     End local timestamp of the task
 * @apiSuccess {String} tasks.data.runtime Duration of the task in seconds
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *        "workflow": "ms2",
 *        "tasks": [
 *           ...
 *           {
 *              "task": "T2.1",
 *              "data": {
 *                 "start": "2016-08-12T15:20:40.631",
 *                 "end": "2016-08-12T15:21:22.205",
 *                 "runtime": 41.574
 *              }
 *           },
 *           {
 *              "task": "T2.2",
 *              "data": {
 *                 "start": "2016-08-12T15:21:46.975",
 *                 "end": "2016-08-12T15:22:25.983",
 *                 "runtime": 39.008
 *              }
 *           },
 *           ...
 *        ],
 *        "start": "2016-08-12T15:17:46.731",
 *        "end": "2016-08-12T15:25:30.452",
 *        "total_runtime": 463.721
 *     }
 *
 * @apiError InternalServerError No results found.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 500 Internal Server Error
 *     {
 *       "error": "No results found."
 *     }
 */
router.get('/:workID/:expID', function(req, res, next) {
    var client = req.app.get('elastic'),
        workflow = req.params.workID,
        experiment = req.params.expID,
        size = 1000,
        json = [];

    workflow = workflow.toLowerCase();

    client.get({
        index: 'mf',
        type: 'workflows',
        id: workflow
    }, function(err, result) {
        if (err) {
            res.status(500);
            return next(err);
        }
        if (result !== undefined) {
            var es_result = {};
            es_result.workflow = workflow;
            var earliest_start = "2200-01-01T00:00:00.000";
            var latest_end = 0;

            var tasks = result._source.tasks;
            es_result.tasks = [];
            async.each(tasks, function(task, callback) {
                    task = task.name;
                    var resource = workflow + "_" + task;
                    resource = resource.toLowerCase();

                    client.search({
                        index: resource,
                        type: experiment,
                        size: 1,
                        sort: ["local_timestamp:asc"],
                    }, function(err, result) {
                        var start;
                        var end;

                        if (err) {
                            res.status(500);
                            return next(err);
                        } else {
                            if (result.hits !== undefined) {
                                var only_results = result.hits.hits;
                                var keys = Object.keys(only_results);
                                keys.forEach(function(key) {
                                    var metric_data = only_results[key]._source;
                                    start = metric_data['local_timestamp'];
                                    start = start.replace(/\s/g, '0');
                                    if (new Date(start) < new Date(earliest_start)) {
                                        earliest_start = start;
                                    }
                                });
                            }
                        }

                        client.search({
                            index: resource,
                            type: experiment,
                            size: 1,
                            sort: ["local_timestamp:desc"],
                        }, function(err, result) {
                            var hostname;

                            if (err) {
                                res.status(500);
                                return next(err);
                            } else {
                                if (result.hits !== undefined) {
                                    var only_results = result.hits.hits;
                                    var keys = Object.keys(only_results);
                                    keys.forEach(function(key) {
                                        var metric_data = only_results[key]._source;
                                        host = metric_data.host;
                                        end = metric_data['local_timestamp'];
                                        end = end.replace(/\s/g, '0');
                                        if (new Date(end) > new Date(latest_end)) {
                                            latest_end = end;
                                        }
                                    });
                                }
                            }

                            var json = {};
                            json.task = task;
                            json.host = hostname;
                            json.data = {};
                            json.data.start = start;
                            json.data.end = end;
                            json.data.runtime = ((new Date(end) - new Date(start))) / 1000;
                            if (!json.data.runtime) {
                                json.data.runtime = 0;
                            }
                            es_result.tasks.push(json);

                            callback();
                        });
                    });
                },
                function(err) {
                    var total_runtime = ((new Date(latest_end) - new Date(earliest_start))) / 1000;
                    es_result.start = earliest_start;
                    es_result.end = latest_end;
                    es_result.total_runtime = total_runtime;
                    res.send(es_result);
                }
            );
        } else {
            res.status(500);
            return next(err);
        }
    });
});

/**
 * @api {get} /runtime/:workflowID/:taskID/:experimentID 2. Get runtime information with given workflow ID, task ID and experiment ID
 * @apiVersion 1.0.0
 * @apiName GetRuntime
 * @apiGroup Runtime
 *
 * @apiParam {String} workflowID      Identifier of a workflow
 * @apiParam {String} taskID          Identifier of a task
 * @apiParam {String} experimentID    Identifier of an experiment
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/runtime/ms2/t1/AVSbT0ChGMPeuCn4QYjq
 *
 * @apiSuccess {String} start     Start timestamp of the specific task and experiment
 * @apiSuccess {String} end       End timestamp of the specific task and experiment
 * @apiSuccess {String} runtime   Duration of the experiment in seconds
 * @apiSuccess {String} host      Hostname of the system
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *          "start":"2016-05-10T17:35:49.125",
 *          "end":"2016-05-10T17:36:01.749",
 *          "runtime":12.624000072479248,
 *          "host":"node01.excess-project.eu"
 *     }
 *
 * @apiError InternalSeverError No results found.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 500 Internal Sever Error
 *     {
 *       "error": "No results found."
 *     }
 */
router.get('/:workID/:taskID/:expID', function(req, res, next) {
    var client = req.app.get('elastic'),
        workflow = req.params.workID.toLowerCase(),
        task = req.params.taskID.toLowerCase(),
        experiment = req.params.expID;

    var index = workflow + '_' + task;

    return client.search({
        index: index,
        type: experiment,
        size: 1,
        sort: ["local_timestamp:asc"],
    }, function(err, result) {
        var start;
        var end;
        var start_original;
        var end_original;

        if (err) {
            res.status(500);
            return next(err);
        } else {
            if (result.hits !== undefined) {
                var only_results = result.hits.hits;
                var keys = Object.keys(only_results);
                keys.forEach(function(key) {
                    var metric_data = only_results[key]._source;
                    start = metric_data['local_timestamp'];
                    start_original = start;
                    start = start.replace(/\s/g, '0');
                    start = new Date(start);
                    start = start.getTime() / 1000;
                });
            }
        }

        client.search({
            index: index,
            type: experiment,
            size: 1,
            sort: ["local_timestamp:desc"],
        }, function(err, result) {
            var host;
            var response;
            if (err) {
                res.status(500);
                return next(err);
            } else {
                if (result.hits !== undefined) {
                    var only_results = result.hits.hits;
                    var keys = Object.keys(only_results);
                    keys.forEach(function(key) {
                        var metric_data = only_results[key]._source;
                        host = metric_data.host;
                        end = metric_data['local_timestamp'];
                        end_original = end;
                        end = end.replace(/\s/g, '0');
                        end = new Date(end);
                        end = end.getTime() / 1000;
                    });
                }
            }

            response = {};
            response.start = start_original;
            response.end = end_original;
            response.runtime = (end - start);
            response.host = host;
            res.send(response);
        });
    });
});

module.exports = router;
