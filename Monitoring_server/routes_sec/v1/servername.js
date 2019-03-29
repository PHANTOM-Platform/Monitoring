var express = require('express'); 
var router = express.Router();

/* GET server name. */
router.get('/servername', function(req, res, next) {
	var json = {};
	json.message = "PHANTOM Monitoring Server";
	res.json(json);
});

module.exports = router;
