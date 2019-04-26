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

var es_servername = "localhost";
var es_port = "9400";
var SERVERDB = "mf";
const contentType_text_plain = 'text/plain';


/* monitoring routes */
var servername      = require('./routes/v1/servername');
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
app.set('version', '29.03.19');
var port = '3033',
hostname = os.hostname();
// redirect backend hostname to front-end
//hostname = hostname.replace('be.excess-project.eu', 'mf.excess-project.eu');
app.set('mf_server', 'http://' + hostname + ':' + port + '/v1');
//app.set('pwm_idx', 'power_dreamcloud');

//****************************************************
function find_param(body, query){
	try{
		if (body != undefined){ //if defined as -F parameter
			return body;
		}else if (query != undefined){ //if defined as ? parameter
			return query;
		}
	}catch(e){
		if (query != undefined){ //if defined as ? parameter
			return query;
		}
	}
	return undefined;
}
//This function is used to confirm that an user exists or not in the DataBase.
function query_count_logs(es_server, my_index, user){
	var my_type = 'logs';
	return new Promise( (resolve,reject) => {
		var elasticsearch = require('elasticsearch');
		var client = new elasticsearch.Client({
			host: es_server,
			log: 'error'
		});
		user="";
// 		if(user==undefined){
// 			user="";
// 		}
		if(user.length==0){
			client.count({
				index: my_index,
				type: my_type,
				body:{"query":{"match_all": {} }, "sort": { "date": { "order": "desc" }}}
			}, function(error, response) {
				if (error) {
					reject (error);
				}else if (response.count !== undefined) {
// 					console.log("response.count is "+response.count+"\n");
					resolve (response.count);//size
				}else{
					resolve (0);//size
				}
			});
		}else{
			client.count({
				index: my_index,
				type: my_type,
				body: {
					"query":{"bool":{"must":[
						{"match_phrase":{"user": user }}
					]}}
				}
			}, function(error, response) {
				if (error) {
					reject (error);
				}else if (response.count !== undefined) {
					resolve (response.count);//size
				}else{
					resolve (0);//size
				}
			});
		}
	});
}//end query_count_project
//****************************************************
//This function is used to confirm that a project exists or not in the DataBase.
//We first counted if existence is >0
function find_logs(es_server, my_index, user, pretty, mysorttype){
	var filter = [ { "date": { "order": "desc" }}];
if (mysorttype!=undefined){
	mysorttype=mysorttype %10;
	if (mysorttype== "2"){//Code
		filter = [ {"code":{ "order":"asc"}}, { "date": { "order": "desc" }} ];
	} else if (mysorttype== "3"){//User
		filter = [ {"user":{ "order":"asc"}}, { "date": { "order": "desc" }} ];
	} else if (mysorttype== "4"){//Ip
		filter = [ {"ip":{ "order":"asc"}}, { "date": { "order": "desc" }} ];
	} else if (mysorttype== "5"){//Message
		filter = [ {"message":{ "order":"asc"}}, { "date": { "order": "desc" }} ];
	}
}else{
	mysorttype=6;
}
	var my_type = 'logs';
	return new Promise( (resolve,reject) => {
		var elasticsearch = require('elasticsearch');
		var client = new elasticsearch.Client({
			host: es_server,
			log: 'error'
		});
		user="";
		if(user.length==0){
			var myquery =  {"query":{"match_all": {} }, "sort": filter };
			if (mysorttype== "1"){//_id
				myquery =  { "query": { "match_all": {} }, "sort": { "_uid": "desc" }, "size": 1 };
			}
			client.search({
				index: my_index,
				type: my_type,
				size: 1000,
				body:myquery
			}, function(error, response) {
				if (error) {
					reject("error: "+error);
				}else{
					resolve( (JSON.stringify(response.hits.hits)));
				}
			});
		}else{
			client.search({
				index: my_index,
				type: my_type,
				size: 1000,
				body: {
					"query":{"bool":{"must":[
						{"match_phrase":{"user": user }}
					]}}
				}
			}, function(error, response) {
				if (error) {
					reject("error: "+error);
				}else{
					resolve( (JSON.stringify(response.hits.hits)));
				}
			});
		}
	});
}
//*********************************************************
function retrieve_file(filePath,res){
	var fs = require('fs');
	var path = require('path');
	var extname = path.extname(filePath);
	var contentType = 'text/html';
	switch (extname) {
		case '.html':
			contentType = 'text/html';
			break;
		case '.js':
			contentType = 'text/javascript';
			break;
		case '.css':
			contentType = 'text/css';
			break;
		case '.json':
			contentType = 'application/json';
			break;
		case '.png':
			contentType = 'image/png';
			break;
		case '.jpg':
			contentType = 'image/jpg';
			break;
		case '.wav':
			contentType = 'audio/wav';
			break;
	}
	fs.readFile(filePath, function(error, content) {
		if (error) {
			if(error.code == 'ENOENT'){
				fs.readFile('./404.html', function(error, content) {
					res.writeHead(404, { 'Content-Type': contentType });
					res.end(content+ "..", 'utf-8');
				});
			} else {
				res.writeHead(500);
				res.end('Sorry, check with the site admin for error: '+error.code+' ..\n');
				res.end(); 
			}
		} else {
			res.writeHead(200, { 'Content-Type': contentType });
			res.end(content, 'utf-8');
		}
	});
}

//**********************************************************
app.get('/servername', function(req, res, next) {
	var SERVERNAME = "PHANTOM Monitoring Server is up and running.";
	res.end(SERVERNAME);
});

//**********************************************************
app.get('/verify_es_connection', function(req, res) {
	var testhttp = require('http');
	testhttp.get('http://'+es_servername+':'+es_port+'/', function(rescode) {
// 		var int_code= parseInt( rescode.statusCode, 10 );
		res.writeHead(rescode.statusCode, { 'Content-Type': contentType_text_plain });
		res.end(""+rescode.statusCode, 'utf-8');
	}).on('error', function(e) {
// 		console.error(e); //if not reply is expected an ECONNREFUSED ERROR, we return 503 as not available service
		res.writeHead(503, { 'Content-Type': contentType_text_plain });
		res.end("503", 'utf-8');
	});
});

app.post('/new_log', function(req, res) {
	"use strict";
// 	var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
// 	var pretty		= find_param(req.body.pretty, req.query.pretty);
	var log_code	= find_param(req.body.code, req.query.code);
	var log_user	= find_param(req.body.user, req.query.user);
	var log_ip		= find_param(req.body.ip, req.query.ip);
	var log_message	= find_param(req.body.message, req.query.message);
	if(log_code==undefined) log_code="";
	if(log_user==undefined) log_user="";
	if(log_ip==undefined) log_ip="";
	if(log_message==undefined) log_message="";
	var resultlog = register_log(es_servername + ":" + es_port, SERVERDB, log_code, log_ip, log_message, currentdate, log_user);
	resultlog.then((resolve_result) => {
		res.writeHead(200, {"Content-Type": contentType_text_plain});
		res.end("registered log\n", 'utf-8');
		return;
	},(reject_result)=> {
		res.writeHead(reject_result.code, {"Content-Type": contentType_text_plain});
		res.end(reject_result.text+": ERROR register_log\n", 'utf-8');
		return;
	});
});

app.get('/get_log_list', function(req, res) {
	"use strict";
// 	var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");  need define datetime
// 	var pretty		= find_param(req.body.pretty, req.query.pretty);
	var mysorttype	= find_param(req.query.sorttype, req.query.sorttype);//error when req.body undefined ....
// 	var projectname	= CommonModule.remove_quotation_marks(find_param(req.body.project, req.query.project));
// 	if (projectname==undefined) projectname="";
//LogsModule
	
	
	var result_count = query_count_logs(es_servername + ":" + es_port,SERVERDB, res.user);
	result_count.then((resultResolve) => {
		if(resultResolve!=0){//new entry (2) we resister new entry
			//LogsModule
			var result_id = find_logs(es_servername + ":" + es_port,SERVERDB, res.user,"false", mysorttype);
			result_id.then((result_json) => {
				res.writeHead(200, {"Content-Type": contentType_text_plain});
				res.end(result_json);
				return;
			},(result_idReject)=> {
				res.writeHead(408, {"Content-Type": contentType_text_plain});
				res.end("error requesting list of logs", 'utf-8');
				return;
			});
		}else{
			res.writeHead(430, {"Content-Type": contentType_text_plain});	//not put 200 then webpage works
// 			if(projectname.length==0){
				res.end("Empty list of logs" );
// 			}else{
// 				res.end("App \""+projectname+"\" not found");
// 			}
			return;
		}
	},(resultReject)=> {
		res.writeHead(402, {"Content-Type": contentType_text_plain});
		res.end(resultReject + "\n", 'utf-8'); //error counting projects in the DB
// 		var resultlog = LogsModule.register_log(es_servername + ":" + es_port,SERVERDB,400,req.connection.remoteAddress,"ERROR on requesting list of logs",currentdate,res.user);
		return;
	});
});

//**********************************************************

app.get('/monitoringserver.html', function(req, res) {
	var filePath = 'web-monitoring/monitoringserver.html';
	retrieve_file(filePath,res);
});
app.get('/phantom.js', function(req, res) {
	var filePath = 'web-monitoring/phantom.js';
	retrieve_file(filePath,res);
});
app.get('/monitoringserver.js', function(req, res) {
	var filePath = 'web-monitoring/monitoringserver.js';
	retrieve_file(filePath,res);
});

//*******************************
app.get('/favicon.ico', function(req, res) {
	var filePath = 'web-monitoring/favicon.ico';
	retrieve_file(filePath,res);
});

app.get('/phantom.css', function(req, res) {
	var filePath = 'web-monitoring/phantom.css';
	retrieve_file(filePath,res);
});

app.get('/phantom.gif', function(req, res) {
	var filePath = 'web-monitoring/phantom.gif';
	retrieve_file(filePath,res);
});
app.get('/javascript_howto.html', function(req, res) {
	var filePath = 'web-monitoring/javascript_howto.html';
	retrieve_file(filePath,res);
});
app.get('/PleaseEnableJavascript.html', function(req, res) {
	var filePath = 'web-monitoring/PleaseEnableJavascript.html';
	retrieve_file(filePath,res);
});


app.get('/log_list.html', function(req, res) {
	var filePath = 'web-monitoring/log_list.html';
	retrieve_file(filePath,res);
});

//app.use(logger('combined', {
//	skip: function (req, res) { return res.statusCode < 400; }
//}));

app.use(bodyParser.json());
app.use(bodyParser.urlencoded({ extended: false }));
app.use(cookieParser());
app.use(express.static(path.join(__dirname, 'public')));

/* monitoring URL paths */
app.use('/', routes);
app.use('/v1/phantom_mf', routes);
app.use('/servername', servername);
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
// const util = require('util');
// console.log(`post/${util.inspect(req.body,false,null)}`);
	var err = new Error('Not Found');
	next(err);
});

// development error handler
// if (app.get('env') === 'development') {
// app.use(function(err, req, res, next) {
// 	var error = {};
// 	error.error = err;
// 	res.json(error);
// });
// }

// production error handler
// app.use(function(err, req, res, next) {
// 	var error = {};
// 	error.error = err;
// 	res.json(error);
// });

module.exports = app;

