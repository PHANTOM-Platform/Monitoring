var express = require('express');
var router = express.Router();

/**
 * @api {get} /resources 1. Get a list of resources links and avaiable platforms
 * @apiVersion 1.0.0
 * @apiName GetResources
 * @apiGroup RM_Resources
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_rm/resources
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *        "alexlaptop": {
 *           "href": "http://mf.excess-project.eu:3033/v1/phantom_rm/resources/alexlaptop"
 *        },
 *        "movidius": {
 *           "href": "http://mf.excess-project.eu:3033/v1/phantom_rm/resources/movidius"
 *        },
 *        "excesscluster": {
 *           "href": "http://mf.excess-project.eu:3033/v1/phantom_rm/resources/excesscluster"
 *        }
 *     }
 *
 * @apiError InternalSeverError No results found.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 500 Internal Sever Error
 *     {
 *       "error": "No resources found."
 *     }
 */
router.get('/', function(req, res, next) {
    var client = req.app.get('elastic'),
        mf_server = req.app.get('mf_server'),
        size = 1000,
        json = {};

    client.search({
        index: 'mf',
        type: 'resources',
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
            json.error = "No resources found.";
            res.json(json);
            return;
        }

        client.search({
            index: 'mf',
            type: 'resources',
            size: size
        }, function(error, response) {
            if (error) {
                res.status(500);
                return next(error);
            }
            if (response.hits !== undefined) {
                var results = response.hits.hits;
                json = get_resource(mf_server, results);
            }
            res.json(json);
        });
    });
});

function get_resource(mf_server, results) {
    var keys = Object.keys(results),
      platform = '',
      response = {};
    keys.forEach(function(key) {
        platform = results[key]._id;
        var json = {};
        json.href = mf_server + '/phantom_rm/resources/' + platform;
        response[platform] = json;
    });
    return response;
}

/**
 * @api {get} /resources/:platformID 2. Get resource information for a given platform
 * @apiVersion 1.0.0
 * @apiName GetResourcesByPlatformID
 * @apiGroup RM_Resources
 *
 * @apiParam {String} platformID                  Unique platform identifier
 * 
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_rm/resources/excesscluster
 *
 * @apiSuccessExample Success-Response:
 *     HTTP/1.1 200 OK
 *     {
 *        "nodes": [
 *           {
 *              "id": "node01",
 *              "cpus": [
 *                 {
 *                    "id": "cpu0",
 *                    "cores": [
 *                       {
 *                          "id": "core0",
 *                          "pwMode": 0,
 *                          "status": "allocated",
 *                          "availTime": "2016-10-21T14:56:02.304"
 *                       },
 *                       {
 *                          "id": "core1",
 *                          "pwMode": 0,
 *                          "status": "allocated",
 *                          "availTime": "2016-10-21T15:21:07.567"
 *                       },
 *                       ...
 *                    ]
 *                 }
 *              ]
 *           }
 *        ]
 *     }
 *
 * @apiError PlatformNotAvailable Given platformID does not exist.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 404 Not Found
 *     {
 *       "error": "Configuration for the platform is not found."
 *     }
 */
router.get('/:platformID', function(req, res, next) {
    var client = req.app.get('elastic'),
      id = req.params.platformID.toLowerCase(),
      json = {};

    client.get({
        index: 'mf',
        type: 'resources',
        id: id
    }, function(error, response) {
        if (error) {
            json.error = "Resources for the platform '" + id + "' is not found.";
        }
        else {
            json = response._source;
        }
        res.json(json);
    });
});

/**
 * @api {put} /resources/:platformID 3. Add/Update resource information for a given platform
 * @apiVersion 1.0.0
 * @apiName PutResources
 * @apiGroup RM_Resources
 *
 * @apiParam {String} platformID         Unique platform identifier
 * @apiParam {String} [id]               Unique identifier of the resource
 * @apiParam {String} [pwMode]           Power mode of the resource
 * @apiParam {String} [status]           Status of the resource (allocated/free)
 * @apiParam {String} [availTime]        Until when the resource will be free
 *
 * @apiExample {curl} Example usage:
 *     curl -i http://mf.excess-project.eu:3033/v1/phantom_rm/resources/excesscluster
 *
 * @apiParamExample {json} Request-Example:
 *     {
 *        "nodes": [
 *           {
 *              "id": "node01",
 *              "cpus": [
 *                 {
 *                    "id": "cpu0",
 *                    "cores": [
 *                       {
 *                          "id": "core0",
 *                          "pwMode": 0,
 *                          "status": "allocated",
 *                          "availTime": "2016-10-21T14:56:02.304"
 *                       },
 *                       {
 *                          "id": "core1",
 *                          "pwMode": 0,
 *                          "status": "allocated",
 *                          "availTime": "2016-10-21T15:21:07.567"
 *                       },
 *                       ...
 *                    ]
 *                 }
 *              ]
 *           }
 *        ]
 *     }
 *
 * @apiError StorageError Given resources could not be stored.
 *
 * @apiErrorExample Error-Response:
 *     HTTP/1.1 500 Internal Server Error
 *     {
 *       "error": "Could not change the resources."
 *     }
 */
router.put('/:platformID', function(req, res, next) {
    var mf_server = req.app.get('mf_server'),
      id = req.params.platformID.toLowerCase(),
      client = req.app.get('elastic'),
      json = {};

    client.index({
        index: 'mf',
        type: 'resources',
        id: id,
        body: req.body
    },function(error, response) {
        if (error) {
            res.status(500);
            return next(error);
        } else {
            json.href = mf_server + '/phantom_rm/resources/' + id;
        }
        res.json(json);
    });
});

module.exports = router;
