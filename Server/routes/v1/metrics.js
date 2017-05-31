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
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/metrics/ms2/t1/AVNXMXcvGMPeuCn4bMe0
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
      workflow = req.params.workflowID.toLowerCase(),
      task = req.params.taskID.toLowerCase(),
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
              for (var item in items) {
                  metrics[item] = item;
              }
          });
      }

      async.each(metrics, function(metric, inner_callback) {
          client.search({
              index: index,
              type: experiment,
              searchType: 'count',
              body: aggregation_by(metric)
            }, function(error, response) {
              if (error) {
                inner_callback(null);
              }
              var aggs = response.aggregations;
                data[metric] = aggs[metric + '_Stats'];
              inner_callback(null);
            });
          }, function() {
            json.push(data);
            res.json(json);
      });
    });
});

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
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/metrics
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
 *       "http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/ms2/t2.1/AVUWnydqGMPeuCn4l-cj",
 *       "http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/ms2/t2.2/AVNXMXcvGMPeuCn4bMe0"
 *     ]
 *
 * @apiError DatabaseError Elasticsearch specific error message.
 */
router.post('/', function(req, res, next) {
    var data = req.body,
      mf_server = req.app.get('mf_server'),
      client = req.app.get('elastic'),
      bulk_data = [];

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

        action.index._index = index.toLowerCase();
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
        for (var i in response.items) {
            json.push(mf_server + '/phantom_mf/profiles/' +
              response.items[i].create._index.replace('_all', '/all') +
              '/' + response.items[i].create._type);
        }
        res.json(json);
    });
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
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_mf/metrics/ms2/t1/AVNXMXcvGMPeuCn4bMe0
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
 *         "href": "http://mf.excess-project.eu:3033/v1/phantom_mf/profiles/ms2/t1/AVNXMXcvGMPeuCn4bMe0"
 *       }
 *     }
 *
 * @apiError DatabaseError Elasticsearch specific error message.
 */
router.post('/:workflowID/:taskID/:experimentID', function(req, res, next) {
    var workflowID = req.params.workflowID.toLowerCase(),
      experimentID = req.params.experimentID,
      taskID = req.params.taskID.toLowerCase(),
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
