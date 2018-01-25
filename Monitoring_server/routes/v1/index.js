var express = require('express');
var dateFormat = require('dateformat');
var router = express.Router();

/* GET home page. */
router.get('/', function(req, res, next) {
    var json = {};
    json.message = "PHANTOM monitoring server is up and running."
    json.release = req.app.get('version');
    json.versions = [ 'v1' ];
    json.current_time = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
    res.json(json);
});

module.exports = router;
