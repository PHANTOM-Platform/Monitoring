var express = require('express');
var router = express.Router();

/**
 * @api {get} /statistics/:workflowID 1. Get statistics of a metric across all tasks and experiments with given workflow ID
 * @apiVersion 1.0.0
 * @apiName GetStats
 * @apiGroup Statistics
 *
 * @apiParam {String} workflowID    Identifier of a workflow
 * @apiParam {String} metric        Name of a metric, e.g., metric=CPU0:core1
 * @apiParam {String} [host]        Hostname of the system, e.g., host=node01
 * @apiParam {String} [from]        Start time of the statistics, e.g., from=2016-05-10T17:35:57.610
 * @apiParam {String} [to]          End time of the statistics, e.g., to=2016-05-10T17:35:57.610
 *
 * @apiExample {curl} Example usage:
 *     curl -i 'http://mf.excess-project.eu:3033/v1/phantom_mf/statistics/ms2?metric=CPU0:core1'
 *
 * @apiSuccess {Object} workflow                     workflow-related data
 * @apiSuccess {String} workflow.href                link to the stored workflow information
 * @apiSuccess {String} metric                       name of the metric for which statistics are captured
 * @apiSuccess {Object} statistics                   extended set of statistics as provided by Elasticsearch
 * @apiSuccess {Number} statistics.count             number of metric values sampled
 * @apiSuccess {Number} statistics.min               minimum value obtained for the given metric
 * @apiSuccess {Number} statistics.max               maximum value obtained for the given metric
 * @apiSuccess {Number} statistics.avg               average value across all metric values
 * @apiSuccess {Number} statistics.sum               sum of all sampled metric values
 * @apiSuccess {Number} statistics.sum_of_squares    sum of squares for the given metric values
 * @apiSuccess {Number} statistics.variance          variance of the given metric
 * @apiSuccess {Number} statistics.std_deviation     standard deviation computed for the given metric
 * @apiSuccess {Object} statistics.std_deviation_bounds          deviation bounds of the given metric
 * @apiSuccess {Number} statistics.std_deviation_bounds.upper    deviation upper bounds
 * @apiSuccess {Number} statistics.std_deviation_bounds.lower    deviation lower bounds
 * @apiSuccess {Object} min                          minimum point of the metrics data
 * @apiSuccess {String} min.local_timestamp          local time, when the the minimum metric value is sampled
 * @apiSuccess {String} min.server_timestamp         server time, when the the minimum metric value is received by the server
 * @apiSuccess {String} min.host                     hostname of the target platform
 * @apiSuccess {String} min.TaskID                   identifier of the task
 * @apiSuccess {String} min.type                     type of plug-in the metric is associated with
 * @apiSuccess {String} min.metric                   metric value associated with a given metric name
 * @apiSuccess {Object} max                          maximum point of the metrics data
 * @apiSuccess {String} max.local_timestamp          local time, when the the maximum metric value is sampled
 * @apiSuccess {String} min.server_timestamp         server time, when the the maximum metric value is received by the server
 * @apiSuccess {String} max.host                     hostname of the target platform
 * @apiSuccess {String} max.TaskID                   identifier of the task
 * @apiSuccess {String} max.type                     type of plug-in the metric is associated with
 * @apiSuccess {String} max.metric                   metric value associated with a given metric name
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *        "workflow": {
 *           "href": "http://mf.excess-project.eu:3033/v1/phantom_mf/workflows/ms2"
 *        },
 *        "metric": "CPU0:core1",
 *        "statistics": {
 *           "count": 1272,
 *           "min": 25,
 *           "max": 30,
 *           "avg": 27.19575471698,
 *           "sum": 34593,
 *           "sum_of_squares": 941499,
 *           "variance": 0.56309518,
 *           "std_deviation": 0.750396685173416,
 *           "std_deviation_bounds": {
 *              "upper": 28.69654808732796,
 *              "lower": 25.6949613466343
 *           }
 *        },
 *        "min": {
 *           "@timestamp": "2016-05-17T16:25:48.123",
 *           "host": "node01.excess-project.eu",
 *           "task": "t1",
 *           "type": "performance",
 *           "CPU0:core0": 26,
 *           "CPU0:core1": 25,
 *           ...
 *        },
 *        "max": {
 *           ...
 *        }
 *     }
 *
 * @apiError NoResults response is empty for the metric.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 200 OK
 *     {
 *       "error": "response is empty for the metric."
 *     }
 */
router.get('/:workflowID', function(req, res, next) {
    var workflowID = req.params.workflowID.toLowerCase(),
        index = workflowID + '_*';

    return handle_response(req, res, next, index);
});

/**
 * @api {get} /statistics/:workflowID/:taskID 2. Get statistics of a metric across all experiments with given workflow ID and task ID
 * @apiVersion 1.0.0
 * @apiName GetStatsTask
 * @apiGroup Statistics
 *
 * @apiParam {String} workflowID    Identifier of a workflow
 * @apiParam {String} taskID        Identifier of a task
 * @apiParam {String} metric        Name of a metric, e.g., metric=CPU0:core1
 * @apiParam {String} [host]        Hostname of the system, e.g., host=node01
 * @apiParam {String} [from]        Start time of the statistics, e.g., from=2016-05-10T17:35:57.610
 * @apiParam {String} [to]          End time of the statistics, e.g., to=2016-05-10T17:35:57.610
 *
 * @apiExample {curl} Example usage:
 *     curl -i 'http://mf.excess-project.eu:3033/v1/phantom_mf/statistics/ms2/t1?metric=metric=CPU0:core1&from=2016-05-10T17:35:57.610&to=2016-05-10T17:36:57.610'
 *
 * @apiSuccess {Object} workflow                     workflow-related data
 * @apiSuccess {String} workflow.href                link to the stored workflow information
 * @apiSuccess {String} metric                       name of the metric for which statistics are captured
 * @apiSuccess {Object} statistics                   extended set of statistics as provided by Elasticsearch
 * @apiSuccess {Number} statistics.count             number of metric values sampled
 * @apiSuccess {Number} statistics.min               minimum value obtained for the given metric
 * @apiSuccess {Number} statistics.max               maximum value obtained for the given metric
 * @apiSuccess {Number} statistics.avg               average value across all metric values
 * @apiSuccess {Number} statistics.sum               sum of all sampled metric values
 * @apiSuccess {Number} statistics.sum_of_squares    sum of squares for the given metric values
 * @apiSuccess {Number} statistics.variance          variance of the given metric
 * @apiSuccess {Number} statistics.std_deviation     standard deviation computed for the given metric
 * @apiSuccess {Object} statistics.std_deviation_bounds          deviation bounds of the given metric
 * @apiSuccess {Number} statistics.std_deviation_bounds.upper    deviation upper bounds
 * @apiSuccess {Number} statistics.std_deviation_bounds.lower    deviation lower bounds
 * @apiSuccess {Object} min                          minimum point of the metrics data
 * @apiSuccess {String} min.local_timestamp          local time, when the the minimum metric value is sampled
 * @apiSuccess {String} min.server_timestamp         server time, when the the minimum metric value is received by the server
 * @apiSuccess {String} min.host                     hostname of the target platform
 * @apiSuccess {String} min.TaskID                   identifier of the task
 * @apiSuccess {String} min.type                     type of plug-in the metric is associated with
 * @apiSuccess {String} min.metric                   metric value associated with a given metric name
 * @apiSuccess {Object} max                          maximum point of the metrics data
 * @apiSuccess {String} max.local_timestamp          local time, when the the maximum metric value is sampled
 * @apiSuccess {String} min.server_timestamp         server time, when the the maximum metric value is received by the server
 * @apiSuccess {String} max.host                     hostname of the target platform
 * @apiSuccess {String} max.TaskID                   identifier of the task
 * @apiSuccess {String} max.type                     type of plug-in the metric is associated with
 * @apiSuccess {String} max.metric                   metric value associated with a given metric name
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *        "workflow": {
 *           "href": "http://mf.excess-project.eu:3033/v1/phantom_mf/workflows/ms2"
 *        },
 *        "metric": "CPU0:core1",
 *        "statistics": {
 *           "count": 1272,
 *           "min": 25,
 *           "max": 30,
 *           "avg": 27.19575471698,
 *           "sum": 34593,
 *           "sum_of_squares": 941499,
 *           "variance": 0.56309518,
 *           "std_deviation": 0.750396685173416,
 *           "std_deviation_bounds": {
 *              "upper": 28.69654808732796,
 *              "lower": 25.6949613466343
 *           }
 *        },
 *        "min": {
 *           "@timestamp": "2016-05-17T16:25:48.123",
 *           "host": "node01.excess-project.eu",
 *           "task": "t1",
 *           "type": "performance",
 *           "CPU0:core0": 26,
 *           "CPU0:core1": 25,
 *           ...
 *        },
 *        "max": {
 *           ...
 *        }
 *     }
 *
 * @apiError NoResults response is empty for the metric.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 200 OK
 *     {
 *       "error": "response is empty for the metric."
 *     }
 */
router.get('/:workflowID/:taskID', function(req, res, next) {
    var workflowID = req.params.workflowID.toLowerCase(),
      taskID = req.params.taskID.toLowerCase(),
      index = workflowID + '_' + taskID;

    return handle_response(req, res, next, index);
});

/**
 * @api {get} /statistics/:workflowID/:taskID/:experimentID 3. Get statistics of a metric with given workflow ID, task ID and experiment ID
 * @apiVersion 1.0.0
 * @apiName GetStatsExperiment
 * @apiGroup Statistics
 *
 * @apiParam {String} workflowID    Identifier of a workflow
 * @apiParam {String} taskID        Identifier of a task
 * @apiParam {String} experimentID  Identifier of an experiment
 * @apiParam {String} metric        Name of a metric, e.g., metric=CPU0:core1
 * @apiParam {String} [host]        Hostname of the system, e.g., host=node01
 * @apiParam {String} [from]        Start time of the statistics, e.g., from=2016-05-10T17:35:57.610
 * @apiParam {String} [to]          End time of the statistics, e.g., to=2016-05-10T17:35:57.610
 *
 * @apiExample {curl} Example usage:
 *     curl -i 'http://mf.excess-project.eu:3033/v1/phantom_mf/statistics/ms2/t1/AVqkW4L57rO13ZBQKOWJ?metric=metric=CPU0:core1'
 *
 * @apiSuccess {Object} workflow                     workflow-related data
 * @apiSuccess {String} workflow.href                link to the stored workflow information
 * @apiSuccess {String} metric                       name of the metric for which statistics are captured
 * @apiSuccess {Object} statistics                   extended set of statistics as provided by Elasticsearch
 * @apiSuccess {Number} statistics.count             number of metric values sampled
 * @apiSuccess {Number} statistics.min               minimum value obtained for the given metric
 * @apiSuccess {Number} statistics.max               maximum value obtained for the given metric
 * @apiSuccess {Number} statistics.avg               average value across all metric values
 * @apiSuccess {Number} statistics.sum               sum of all sampled metric values
 * @apiSuccess {Number} statistics.sum_of_squares    sum of squares for the given metric values
 * @apiSuccess {Number} statistics.variance          variance of the given metric
 * @apiSuccess {Number} statistics.std_deviation     standard deviation computed for the given metric
 * @apiSuccess {Object} statistics.std_deviation_bounds          deviation bounds of the given metric
 * @apiSuccess {Number} statistics.std_deviation_bounds.upper    deviation upper bounds
 * @apiSuccess {Number} statistics.std_deviation_bounds.lower    deviation lower bounds
 * @apiSuccess {Object} min                          minimum point of the metrics data
 * @apiSuccess {String} min.local_timestamp          local time, when the the minimum metric value is sampled
 * @apiSuccess {String} min.server_timestamp         server time, when the the minimum metric value is received by the server
 * @apiSuccess {String} min.host                     hostname of the target platform
 * @apiSuccess {String} min.TaskID                   identifier of the task
 * @apiSuccess {String} min.type                     type of plug-in the metric is associated with
 * @apiSuccess {String} min.metric                   metric value associated with a given metric name
 * @apiSuccess {Object} max                          maximum point of the metrics data
 * @apiSuccess {String} max.local_timestamp          local time, when the the maximum metric value is sampled
 * @apiSuccess {String} min.server_timestamp         server time, when the the maximum metric value is received by the server
 * @apiSuccess {String} max.host                     hostname of the target platform
 * @apiSuccess {String} max.TaskID                   identifier of the task
 * @apiSuccess {String} max.type                     type of plug-in the metric is associated with
 * @apiSuccess {String} max.metric                   metric value associated with a given metric name
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *        "workflow": {
 *           "href": "http://mf.excess-project.eu:3033/v1/phantom_mf/workflows/ms2"
 *        },
 *        "metric": "CPU0:core1",
 *        "statistics": {
 *           "count": 1272,
 *           "min": 25,
 *           "max": 30,
 *           "avg": 27.19575471698,
 *           "sum": 34593,
 *           "sum_of_squares": 941499,
 *           "variance": 0.56309518,
 *           "std_deviation": 0.750396685173416,
 *           "std_deviation_bounds": {
 *              "upper": 28.69654808732796,
 *              "lower": 25.6949613466343
 *           }
 *        },
 *        "min": {
 *           "@timestamp": "2016-05-17T16:25:48.123",
 *           "host": "node01.excess-project.eu",
 *           "task": "t1",
 *           "type": "performance",
 *           "CPU0:core0": 26,
 *           "CPU0:core1": 25,
 *           ...
 *        },
 *        "max": {
 *           ...
 *        }
 *     }
 *
 *
 * @apiError NoResults response is empty for the metric.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 200 OK
 *     {
 *       "error": "response is empty for the metric."
 *     }
 */
router.get('/:workflowID/:taskID/:experimentID', function(req, res, next) {
    var workflowID = req.params.workflowID.toLowerCase(),
      taskID = req.params.taskID.toLowerCase(),
      type = req.params.experimentID,
      index = workflowID + '_' + taskID;

    return handle_response(req, res, next, index, type);
});

function handle_response(req, res, next, index, type) {
    var client = req.app.get('elastic'),
      mf_server = req.app.get('mf_server') + '/phantom_mf',
      workflowID = req.params.workflowID.toLowerCase(),
      host = req.query.host,
      metric = req.query.metric,
      from = req.query.from,
      to = req.query.to,
      body = aggregation_by(metric, type);

     if (!is_defined(metric)) {
        var error = {
            'error': {
                'message': 'parameter metric is missing'
            }
        }
        res.json(error);
        return;
    }

    if (is_defined(from) && is_defined(to)) {
        body = filter_and_aggregate_by(metric, from, to, type, host);
    }

    client.indices.refresh({
        index: index
    }, function(error, response) {
        if (error) {
            res.json(error);
            return;
        }
        client.search({
            index: index,
            searchType: 'count',
            body: body
        }, function(error, response) {
            if (error) {
                res.json(error);
                return;
            }
            else {
                if(response.aggregations !== undefined) {
                    var metrics = [],
                        answers = [];
                    if (typeof(metric) == "string") {
                        metrics[0] = metric;
                    }
                    else {
                        metrics = metric;
                    }
                    for (var key in metrics) {
                        var answer = {},
                            aggs = response.aggregations;
                        answer['workflow'] = {};
                        answer['workflow'].href = mf_server + '/workflows/' + workflowID;
                        answer['metric'] = metrics[key];

                        if (is_defined(from) && is_defined(to)) {
                            aggs = aggs['filtered_stats'];
                        }
                        if(aggs['Minimum_' + metrics[key]]['hits']['total'] == 0) {
                                var json = {};
                                json.error = "response is empty for the metric";
                                answers.push(json);
                        }
                        else {
                            answer['statistics'] = aggs[metrics[key] + '_Stats'];
                            answer['min'] = aggs['Minimum_' + metrics[key]]['hits']['hits'][0]['_source'];
                            answer['max'] = aggs['Maximum_' + metrics[key]]['hits']['hits'][0]['_source'];
                            answers.push(answer);
                        }
                    }
                    res.json(answers);        
                }
                else {
                    var json = {};
                    json.error = "index " + index +" is not found in DB";
                    res.json(json);
                }
            }
        });
    });
}

function is_defined(variable) {
    return (typeof variable !== 'undefined');
}

function filter_and_aggregate_by(field_name, from, to, type, host) {
    var filter_type = '';
    if (is_defined(type)) {
        filter_type = '{ ' +
                        '"type": { "value": "' + type + '" }' +
                    '},';
    }
    if(is_defined(host)) {
        filter_type += '{ ' +
                        '"prefix": { "host": "' + host + '" }' +
                    '},';
    }
    return '{' +
        '"aggs": {' +
            '"filtered_stats": {' +
                '"filter": {' +
                    '"and": [' +
                        filter_type +
                        date_filter(from, to) +
                    ']' +
                '},' +
                aggregation_by(field_name).slice(1, -1) +
            '}' +
        '}' +
    '}';
}

function date_filter(from, to) {
    var filter = '';

    if (is_defined(from) && is_defined(to)) {
        filter =
            '{ ' +
                '"range": {' +
                    '"local_timestamp": {' +
                        '"from": "' + from + '",' +
                        '"to": "' + to + '"' +
                    '}' +
                '}' +
            '}';
    }

    return filter;
}

function type_filter(type) {
    var filter = '';

    if (is_defined(type)) {
        filter =
            '"query": {' +
                '"filtered": {' +
                    '"filter": {' +
                        '"type": { "value": "' + type + '" }' +
                    '}' +
                '}' +
            '},';
    }

    return filter;
}

function aggregation_by(field_name, type) {
    var fields = [];
    var query_msg = '{' + type_filter(type) +
        '"aggs": {';
    if(typeof(field_name) == "string") {
        fields[0] = field_name;
    }
    else {
        fields = field_name;
    }
    for (var key in fields) {
        query_msg +=
            '"' + fields[key] + '_Stats" : {' +
                '"extended_stats" : {' +
                    '"field" : "' + fields[key] + '"' +
                '}' +
            '},' +
            '"Minimum_' + fields[key] + '": {' +
                '"top_hits": {' +
                    '"size": 1,' +
                    '"sort": [' +
                        '{' +
                            '"' + fields[key] + '": {' +
                                '"order": "asc"' +
                            '}' +
                        '}' +
                    ']' +
                '}' +
            '},' +
            '"Maximum_' + fields[key] + '": {' +
                '"top_hits": {' +
                    '"size": 1,' +
                    '"sort": [' +
                        '{' +
                            '"' + fields[key] + '": {' +
                                '"order": "desc"' +
                            '}' +
                        '}' +
                    ']' +
                '}' +
            '},'
    }
    query_msg = query_msg.slice(0, -1);
    query_msg +=
        '}' +
    '}';
    return query_msg;
}

module.exports = router;