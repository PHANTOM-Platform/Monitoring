var express = require('express');
var router = express.Router();

/**
 * @api {get} /configs 1. Get a list of configurations of all platforms
 * @apiVersion 1.0.0
 * @apiName GetConfigs
 * @apiGroup RM_Configs
 *
 * @apiSuccess {Object} platformID                    Unique platform identifier
 * @apiSuccess {String} platformID.status             Status of the plugin (on/off)
 * @apiSuccess {String} platformID.sampling_interval  Sampling interval of the plugin (in nanosecond)
 * @apiSuccess {String} platformID.metrics            Name and status (on/off) of metrics of the plugin
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_rm/configs
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *          "node01.excess-cluster":{
 *              "mf_plugin_Board_power":{
 *                  "status":"on",
 *                  "sampling_interval":"1000000000ns",
 *                  "device0:current":"on",
 *                  "device0:vshunt":"on",
 *                  "device0:vbus":"on",
 *                  "device0:power":"on"},
 *              "mf_plugin_CPU_perf":{
 *                  "status":"on",
 *                  "sampling_interval":"1000000000ns",
 *                  "MIPS":"on"},
 *              "mf_plugin_CPU_temperature":{
 *                  "status":"on",
 *                  "sampling_interval":"1000000000ns",
 *                  "CPU0:core0":"off"}
 *          },
 *          "movidius":{
 *              "power":{
 *                  "status":"on",
 *                  "sampling_interval":"1000000000ns"},
 *              "temperature":{
 *                  "status":"on",
 *                  "sampling_interval":"1000000000ns"}
 *          }
 *      }
 *
 * @apiError ConfigsNotAvailable No configurations found.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 404 Not Found
 *     {
 *       "error": "No configurations found."
 *     }
 */
router.get('/', function(req, res, next) {
    var client = req.app.get('elastic'),
        size = 100,
        json = {};

    client.search({
        index: 'mf',
        type: 'configurations',
        searchType: 'count'
    }, function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        }
        if (response.hits !== undefined) {
            size = response.hits.total;
        }
        if (size === 0) {
            res.status(404);
            json.error = "No configurations found.";
            res.json(json);
            return;
        }

        client.search({
            index: 'mf',
            type: 'configurations',
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

function get_details(results) {
    var keys = Object.keys(results),
        response = {};
    keys.forEach(function(key) {
        var source = results[key]._source,
            item = JSON.parse(JSON.stringify(source));
        response[results[key]._id] = item;
    });
    return response;
}

/**
 * @api {get} /configs/:platformID 2. Get the configuration of a specific platform
 * @apiVersion 1.0.0
 * @apiName GetConfigsByPlatformID
 * @apiGroup RM_Configs
 *
 * @apiParam {String} platformID                  Unique platform identifier
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_rm/configs/node01.excess-cluster
 *
 * @apiSuccess (body) {String} status             Status of the plugin (on/off)
 * @apiSuccess (body) {String} sampling_interval  Sampling interval of the plugin (in nanosecond)
 * @apiSuccess (body) {String} metrics            Name and status (on/off) of metrics of the plugin
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *          "mf_plugin_Board_power": {
 *              "status": "on",
 *              "sampling_interval": "1000000000ns",
 *              "device0:current": "on",
 *              "device0:vshunt": "on",
 *              "device0:vbus": "off",
 *              "device0:power": "on" }, 
 *          "mf_plugin_CPU_perf": {
 *              "status": "on",
 *              "sampling_interval": "1000000000ns",
 *              "MIPS": "on" }, 
 *          "mf_plugin_CPU_temperature": {
 *              "status": "on",
 *              "sampling_interval": "1000000000ns",
 *              "CPU0:core0": "on"}
 *      }
 *
 * @apiError PlatformNotAvailable Given platformID does not exist.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 404 Not Found
 *     {
 *       "error": "Configuration for the platform'" + platformID + "' is not found."
 *     }
 */
router.get('/:platformID', function(req, res, next) {
    var client = req.app.get('elastic'),
        platformID = req.params.platformID.toLowerCase(),
        size = 100,
        json = {};

    client.get({
        index: 'mf',
        type: 'configurations',
        id: platformID
    }, function(error, response) {
        if (response.found) {
            json = response._source;
        } else {
            json.error = "Configuration for the platform'" + platformID + "' is not found.";
        }
        res.json(json);
    });
});

/**
 * @api {put} /configs/:platformID 3. Add/Updata the configuration of a specific platform
 * @apiVersion 1.0.0
 * @apiName PutConfigs
 * @apiGroup RM_Configs
 *
 * @apiParam {String} platformID         Unique platform identifier
 * @apiParam {String} status             Status of the plugin (on/off)
 * @apiParam {String} sampling_interval  Sampling interval of the plugin (in nanosecond)
 * @apiParam {String} metrics            Name and status (on/off) of metrics of the plugin
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_rm/configs/node01.excess-cluster
 *
 * @apiParamExample {json} Request-Example:
 *     {
 *          "mf_plugin_Board_power": {
 *              "status": "on",
 *              "sampling_interval": "1000000000ns",
 *              "device0:current": "on",
 *              "device0:vshunt": "on",
 *              "device0:vbus": "off",
 *              "device0:power": "on" }, 
 *          "mf_plugin_CPU_perf": {
 *              "status": "on",
 *              "sampling_interval": "1000000000ns",
 *              "MIPS": "on" }, 
 *          "mf_plugin_CPU_temperature": {
 *              "status": "on",
 *              "sampling_interval": "1000000000ns",
 *              "CPU0:core0": "on"}
 *      }
 *
 * @apiSuccess {String} href Link to the stored configuration resource
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *       "href": "http://mf.excess-project.eu:3033/v1/phantom_rm/configs/node01.excess-cluster",
 *     }
 *
 * @apiError StorageError Given configuration could not be stored.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 500 Internal Server Error
 *     {
 *       "error": "Could not change the configuration."
 *     }
 */
router.put('/:platformID', function(req, res, next) {
    var platformID = req.params.platformID.toLowerCase(),
        mf_server = req.app.get('mf_server'),
        client = req.app.get('elastic'),
        json = {};

    client.index({
        index: 'mf',
        type: 'configurations',
        id: platformID,
        body: req.body
    }, function(error, response) {
        if (error !== 'undefined') {
            json.href = mf_server + '/phantom_rm/configs/' + platformID;
        } else {
            res.status(500);
            json.error = "Could not change the configuration.";
        }
        res.json(json);
    });
});

module.exports = router;