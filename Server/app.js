var express = require('express');
var path = require('path');
var favicon = require('serve-favicon');
var logger = require('morgan');
var cookieParser = require('cookie-parser');
var bodyParser = require('body-parser');
var os = require("os");
var elasticsearch = require('elasticsearch');
var elastic = new elasticsearch.Client({
  host: 'localhost:9400',
  log: 'error'
});

/* monitoring routes */
var routes      = require('./routes/v1/index');
var workflows   = require('./routes/v1/workflows');
var devices     = require('./routes/v1/devices'); /*2017*/
var summary     = require('./routes/v1/summary'); /*2017*/
var deployment = require('./routes/v1/deployment'); /*2017*/
var deploymentcomm = require('./routes/v1/deploymentcomm'); /*2017*/
var deploymentcomp = require('./routes/v1/deploymentcomp'); /*2017*/
var experiments = require('./routes/v1/experiments');
var metrics     = require('./routes/v1/metrics');
var profiles    = require('./routes/v1/profiles');
var runtime     = require('./routes/v1/runtime');
var statistics  = require('./routes/v1/statistics');

/* resource manager routes */
var configs = require('./routes/v1/configs');
var resources = require('./routes/v1/resources');

var app = express();
app.set('views', path.join(__dirname, 'views'));
app.set('view engine', 'jade');
app.set('elastic', elastic);
app.set('version', '23.01.18');
var port = '3033',
  hostname = os.hostname();
// redirect backend hostname to front-end
//hostname = hostname.replace('be.excess-project.eu', 'mf.excess-project.eu');
app.set('mf_server', 'http://' + hostname + ':' + port + '/v1');
//app.set('pwm_idx', 'power_dreamcloud');

app.use(logger('combined', {
  skip: function (req, res) { return res.statusCode < 400; }
}));

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

/* monitoring URL paths */
app.use('/', routes);
app.use('/v1/phantom_mf', routes);
app.use('/v1/phantom_mf/workflows', workflows);
app.use('/v1/phantom_mf/experiments', experiments);
app.use('/v1/phantom_mf/devices',devices); /*2017*/
app.use('/v1/phantom_mf/summary',summary); /*2017*/
app.use('/v1/phantom_mf/deployment',deployment); /*2017*/
app.use('/v1/phantom_mf/deploymentcomm',deploymentcomm); /*2017*/
app.use('/v1/phantom_mf/deploymentcomp',deploymentcomp); /*2017*/


app.use('/v1/phantom_mf/metrics', metrics);
app.use('/v1/phantom_mf/profiles', profiles);
app.use('/v1/phantom_mf/runtime', runtime);
app.use('/v1/phantom_mf/statistics', statistics);

/*resource manager URL paths */
app.use('/v1/phantom_rm/resources', resources);
app.use('/v1/phantom_rm/configs', configs);

/* catch 404 and forward to error handler */
app.use(function(req, res, next) {
  var err = new Error('Not Found');
  next(err);
});

// development error handler
if (app.get('env') === 'development') {
  app.use(function(err, req, res, next) {
    var error = {};
    error.error = err;
    res.json(error);
  });
}

// production error handler
app.use(function(err, req, res, next) {
  var error = {};
  error.error = err;
  res.json(error);
});

module.exports = app;