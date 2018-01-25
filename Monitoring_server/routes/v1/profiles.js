var express = require('express');
var dateFormat = require('dateformat');
var router = express.Router();
var async = require('async');

/**
 * @api {get} /profiles/:workflowID 1. Get a list of the profiled tasks and experiments with given workflow ID
 * @apiVersion 1.0.0
 * @apiName GetProfilesWorkflow
 * @apiGroup Profiles
 *
 * @apiParam {String} workflowID    Identifier of a workflow
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy
 *
 * @apiSuccess {Object} taskID                    Identifier of a registered task
 * @apiSuccess {Object} taskID.experimentID       Identifier of an experiment
 * @apiSuccess {String} taskID.experimentID.href  Link to the experiment
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *       "t1":{
 *          "AVSf5_wVGMPeuCn4Qdw2":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVSf5_wVGMPeuCn4Qdw2"
 *          },
 *          "AVSf-mU4GMPeuCn4Qd0L":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVSf-mU4GMPeuCn4Qd0L"
 *          }
 *       },
 *       "t2":{
 *          "AVXAMB5FLeaeU4rxyi3w":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t2/AVXAMB5FLeaeU4rxyi3w"
 *          },
 *          "AVVT4dhwenoRsEhyDkeb":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t2/AVVT4dhwenoRsEhyDkeb"
 *          }
 *       }
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
router.get('/:workID', function(req, res, next) {
    var client = req.app.get('elastic'),
      mf_server = req.app.get('mf_server'),
      workflow = req.params.workID.toLowerCase(),
      json = {};

    client.indices.getSettings({
        index: workflow + '*',
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        }
        if (response !== undefined) {
            var results = Object.keys(response);

            async.map(results, function(index, callback) {
                var task = index;
                task = task.replace(workflow + '_', '');
                // please remove for production mode
                if (task.indexOf("2015") > -1) {
                    return callback(null, json);
                }

                client.indices.getMapping({
                    index: index,
                }, function(error, response) {
                    if (error === undefined) {
                        var mappings = response[index].mappings;
                        mappings = Object.keys(mappings);
                        if (json[task] === undefined) {
                            json[task] = {};
                        }
                        for (var i in mappings) {
                            var item = {};
                            // remove in prod. mode
                            if (typeof task !== 'undefined') {
                                item.href = mf_server + '/phantom_mf/profiles/' + workflow + '/' + task + '/' + mappings[i];
                                json[task][mappings[i]] = item;
                            } else {
                                item.href = mf_server +  '/phantom_mf/profiles/' + workflow + '/' + mappings[i];
                                json[mappings[i]] = item;
                            }
                        }
                        return callback(null, json);
                    } else {
                        json.error = error;
                    }
                });
            }, function(err, results) {
                // remove in production mode
                if (typeof results[0] == 'undefined') {
                    res.status(500);
                    var msg = {};
                    msg.error = "No results found.";
                    res.json(msg);
                } else {
                    for (var key in results[0]) {
                        if (isEmpty(results[0][key])) {
                            delete results[0][key];
                        }
                    }
                    res.json(results[0]);
                }
            });
        }
    });
});

function isEmpty(obj) {
    var name;
    for (name in obj) {
        return false;
    }
    return true;
}

/**
 * @api {get} /profiles/:workflowID/:taskID 2. Get a list of the profiled experiments with given workflow ID and task ID
 * @apiVersion 1.0.0
 * @apiName GetProfilesTask
 * @apiGroup Profiles
 *
 * @apiParam {String} workflowID     Identifier of a workflow
 * @apiParam {String} taskID         Identifier of a registered task
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1
 *
 * @apiSuccess {Object} date                    Date, when the task is registered
 * @apiSuccess {Object} date.experimentID       Identifier of an experiment
 * @apiSuccess {String} date.experimentID.href  Link to the experiment
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *       "2016-05-11":{
 *          "AVSf5_wVGMPeuCn4Qdw2":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVSf5_wVGMPeuCn4Qdw2"
 *          },
 *          "AVSf-mU4GMPeuCn4Qd0L":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVSf-mU4GMPeuCn4Qd0L"
 *          }
 *       },
 *       "2016-05-10":{
 *          "AVXAMB5FLeaeU4rxyi3w":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVXAMB5FLeaeU4rxyi3w"
 *          },
 *          "AVVT4dhwenoRsEhyDkeb":{
 *                "href":"http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVVT4dhwenoRsEhyDkeb"
 *          }
 *       }
 *     }
 *
 * @apiError InternalSeverError No results found.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 500 Internal Sever Error
 *     {
 *       "error": "Task not found."
 *     }
 */
router.get('/:workID/:taskID', function(req, res, next) {
    var client = req.app.get('elastic'),
      workflow = req.params.workID.toLowerCase(),
      task = req.params.taskID.toLowerCase(),
      mf_server = req.app.get('mf_server'),
      size = 1000,
      json = {};

    // assign default taskID when application has no workflow
    if (!is_defined(task)) {
        res.status(500);
        json.error = "Task not found";
        res.json(json);
        return;
    }
    var index = workflow + '_' + task;
    index = index.toLowerCase();

    client.indices.getMapping({
        index: index,
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        }
        if (response !== undefined) {
            var mappings = response[index].mappings;
            mappings = Object.keys(mappings);
            async.map(mappings, function(experimentID, callback) {
                client.get({
                    index: 'mf',
                    type: 'experiments',
                    id: experimentID,
                    parent: workflow
                }, function(error, response) {
                    if (response.found) {
                        var result = response._source,
                          timestamp = result.timestamp,
                          href = mf_server + '/phantom_mf/profiles/' + workflow + '/' + task + '/' + experimentID;
                        var element = {};
                        element.href = href;
                        if (typeof timestamp !== 'undefined') {
                            timestamp = timestamp.split('-')[0];
                            timestamp = timestamp.replace(/\./g, '-');
                            if (json[timestamp] === undefined) {
                                json[timestamp] = {};
                            }
                        } else if (typeof result['@timestamp'] !== 'undefined') {
                            timestamp = result['@timestamp'].split('T')[0];
                            if (json[timestamp] === undefined) {
                                json[timestamp] = {};
                            }
                        }
                        json[timestamp][experimentID] = element;
                    } else {
                        json.error = error;
                    }
                    return callback(null, json);
                });
            }, function(err, results) {
                if (typeof results[0] == 'undefined') {
                    res.status(500);
                    var msg = {};
                    msg.error = "No results found.";
                    res.json(msg);
                } else {
                    res.json(results[0]);
                }
            });
        }
    });
});

/**
 * @api {get} /profiles/:workflowID/:taskID/:experimentID 3. Get the profiled metrics with given workflow ID, task ID and experiment ID
 * @apiVersion 1.0.0
 * @apiName GetProfilesExperiment
 * @apiGroup Profiles
 *
 * @apiParam {String} workflowID      Identifier of a workflow
 * @apiParam {String} taskID          Identifier of a registered task
 * @apiParam {String} experimentID    Identifier of an experiment
 * @apiParam {String} [from]          Time filter, starting point of the metrics collection time (if not given, starting point is 5 min from current time)
 * @apiParam {String} [to]            Time filter, ending point of the metrics collection time (if not given, ending point is the current time)
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/dummy/t1/AVSbT0ChGMPeuCn4QYjq
 *
 * @apiSuccess {Object} Metrics                     Measurements of an experiment based on the system
 * @apiSuccess {String} Metrics.local_timestamp     Local time, when the metric data is collected
 * @apiSuccess {String} Metrics.server_timestamp    Server time, when the metric data is received by the server
 * @apiSuccess {String} Metrics.host                Hostname of the target system
 * @apiSuccess {String} Metrics.type                Type of the metric, e.g. power, temperature, and so on
 * @apiSuccess {Number} Metrics.metric              Name and value of the metric
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     [
 *         {
 *          "local_timestamp":"2016-05-10T17:35:59.576",
 *          "server_timestamp":"2016-05-10T17:36:01.541",
 *          "host":"node01.excess-project.eu",
 *          "type":"energy",
 *          "DRAM_ENERGY:PACKAGE0":1.5715,
 *          "DRAM_POWER:PACKAGE0":1.571,
 *         },{
 *          "local_timestamp":"2016-05-10T17:35:59.708",
 *          "server_timestamp":"2016-05-10T17:36:01.541",
 *          "host":"node01.excess-project.eu",
 *          "type":"memory",
 *          "MemTotal":32771284,
 *          "MemFree":31720604
 *         },{
 *          "local_timestamp":"2016-05-10T17:35:59.831",
 *          "server_timestamp":"2016-05-10T17:36:01.541",
 *          "host":"node01.excess-project.eu",
 *          "type":"temperature",
 *          "CPU1_Core 1":30,
 *          "CPU1_Core 0":25
 *         }
 *     ]
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
      experiment = req.params.expID,
      filter = req.query.filter,
      from = req.query.from,
      to = req.query.to,
      size = 1000,
      json = [];

    var index = workflow + '_' + task;

    /* if from or to timestamps are not given:
       get the metrics in the last 5 minutes */
    if (!is_defined(from) || !is_defined(to)) {
        time_now = new Date();
        to = dateFormat(time_now, "yyyy-mm-dd'T'HH:MM:ss.l");
        from = dateFormat(new Date(time_now.valueOf() - 5 * 60000), "yyyy-mm-dd'T'HH:MM:ss.l"); // from is 5 minutes before to
    }

    var body = '{"query": {' +
                '"filtered": {' +
                    '"filter": {' +
                        '"range": {' + 
                            '"local_timestamp": {' + 
                                '"from": "' + from + '",' + 
                                '"to": "' + to + '"' +
                                '}' +
                            '}' + 
                        '}' +
                    '}' +
                '}}';

    client.search({
        index: index,
        type: experiment,
        body: body,
        searchType: 'count'
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        }
        if (response.hits !== undefined) {
            size = response.hits.total;
        }

        client.search({
            index: index,
            type: experiment,
            body: body,
            size: size
        }, function(error, response) {
            if (error) {
                res.status(500);
                return next(error);
            }
            if (response.hits !== undefined) {
                var results = response.hits.hits,
                  keys = Object.keys(results),
                  item = {};
                keys.forEach(function(key) {
                    item = results[key]._source;
                    if (typeof item['@timestamp'] !== 'undefined') {
                        item['@timestamp'] = item['@timestamp'].replace(/\s/g, '0');
                    }
                    json.push(item);
                });
            }

            if (typeof json[0] == 'undefined') {
                res.status(500);
                var msg = {};
                msg.error = "No results found.";
                res.json(msg);
            } else {
                res.json(json);
            }
        });
    });
});

function is_defined(variable) {
    return (typeof variable !== 'undefined');
}

module.exports = router;
