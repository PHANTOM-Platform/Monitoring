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


//********************* SUPPORT JS file, for DB functionalities *****
// 	const MetadataModule 	= require('./support-metadata');
	const UsersModule 		= require('./support-usersaccounts');
	const LogsModule 		= require('./support-logs');
	const CommonModule 		= require('./support-common');
//*********************** SUPPORT JS file, for TOKENS SUPPORT *******
	var bodyParser	= require('body-parser');
	var cors		= require('cors');
	var auth		= require('./token-auth');
var middleware	= require('./token-middleware');
	const colours 			= require('./colours');
const ips = ['::ffff:127.0.0.1','127.0.0.1',"::1"];

var es_servername = "localhost";
var es_port = "9400";
var SERVERDB = "mf";
const contentType_text_plain = 'text/plain';


var dateFormat = require('dateformat');

/* monitoring routes */
var servername      = require('./routes_sec/v1/servername');
var routes      = require('./routes_sec/v1/index');
var workflows   = require('./routes_sec/v1/workflows');
var devices     = require('./routes_sec/v1/devices'); /*2017*/
var summary     = require('./routes_sec/v1/summary'); /*2017*/
var deployment = require('./routes_sec/v1/deployment'); /*2017*/
var deploymentcomm = require('./routes_sec/v1/deploymentcomm'); /*2017*/
var deploymentcomp = require('./routes_sec/v1/deploymentcomp'); /*2017*/
var experiments = require('./routes_sec/v1/experiments');
var metrics     = require('./routes_sec/v1/metrics');
var profiles    = require('./routes_sec/v1/profiles');
var runtime     = require('./routes_sec/v1/runtime');
var statistics  = require('./routes_sec/v1/statistics');

/* resource manager routes */
var configs = require('./routes_sec/v1/configs');
var resources = require('./routes_sec/v1/resources');

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
// 		if(user==undefined){ user=""; }
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
function find_logs(es_server, my_index, user){
	var my_type = 'logs';
	return new Promise( (resolve,reject) => {
		var elasticsearch = require('elasticsearch');
		var client = new elasticsearch.Client({
			host: es_server,
			log: 'error'
		});
		user="";
		if(user.length==0){
			client.search({
				index: my_index,
				type: my_type,
				size: 1000,
				body:{"query":{"match_all": {} }, "sort": { "date": { "order": "desc" }}}
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

//**********************************************************
//example:
// curl -H "Content-Type: text/plain" -XPOST http://localhost:8000/signup?name="bob"\&email="bob@abc.commm"\&pw="1234"
// app.post('/signup',ipfilter(ips, {mode: 'allow'}), function(req, res) {
app.post('/signup', function(req, res) {
	"use strict";
	var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l"); 
	if( (req.body==undefined) && (req.query==undefined)){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Missing parameters.\n");
		return;
	}
	if(req.body==undefined) {
		req.body={};
	}
	if(req.query==undefined){
		req.query={};
	}
	var name= find_param(req.body.userid, req.query.userid);
	var email= find_param(req.body.email, req.query.email);
	var pw=find_param(req.body.pw, req.query.pw);
	var resultlog;
	if (pw == undefined){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: SIGNUP Bad Request, missing Passwd.\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400,req.connection.remoteAddress,"SIGNUP Bad Request, missing Passwd",currentdate,"");
		return;
	}else if(pw.length == 0){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: SIGNUP Bad Request, empty Passwd.\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400,req.connection.remoteAddress,"SIGNUP Bad Request, Empty Passwd",currentdate,"");
		return;
	}
	if (email == undefined){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Bad Request, missing Email.\n");
		resultlog = LogsModule.register_log( es_servername+":"+es_port,SERVERDB,400,req.connection.remoteAddress,"SIGNUP Bad Request, missing Email",currentdate,"");
		return;
	}else if (email.length == 0){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Bad Request, Empty Email.\n");
		resultlog = LogsModule.register_log( es_servername+":"+es_port,SERVERDB,400,req.connection.remoteAddress,"SIGNUP Bad Request, Empty Email",currentdate,"");
		return;
	}
	console.log("[LOG]: REGISTER USER+PW ");
	console.log("   " +colours.FgYellow + colours.Bright + " user: " + colours.Reset + email );
	console.log("   " +colours.FgYellow + colours.Bright + " request from IP: " + req.connection.remoteAddress + colours.Reset+"\n");
	if(( req.connection.remoteAddress!= ips[0] ) &&( req.connection.remoteAddress!=ips[1])&&( req.connection.remoteAddress!=ips[2])){
		console.log(" ACCESS DENIED from IP address: "+req.connection.remoteAddress);
		var messagea = "REGISTER USER '"+ email + "' FORBIDDEN access from external IP";
		resultlog = LogsModule.register_log( es_servername+":"+es_port,SERVERDB,403,req.connection.remoteAddress,messagea,currentdate,"");
		res.writeHead(403, {"Content-Type": contentType_text_plain});
		res.end("\n403: FORBIDDEN access from external IP.\n");
		return;
	}
	var result = UsersModule.register_new_user(es_servername+":"+es_port,SERVERDB, name, email, pw);
	result.then((resultreg) => {
		var messageb = "REGISTER USER '"+ email + "' GRANTED";
		resultlog = LogsModule.register_log( es_servername+":"+es_port,SERVERDB,resultreg.code, req.connection.remoteAddress, messageb,currentdate,"");
		var verify_flush = CommonModule.my_flush( req.connection.remoteAddress,es_servername+':'+es_port, SERVERDB);
		verify_flush.then((resolve_result) => {
			res.writeHead(resultreg.code, {"Content-Type": contentType_text_plain});
			res.end("Succeed\n");
		},(reject_result)=> {
			res.writeHead(reject_result.code, {"Content-Type": contentType_text_plain});
			res.end(reject_result.text+": ERROR FLUSH\n", 'utf-8');
			resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, reject_result.code, req.connection.remoteAddress, reject_result.text+"ERROR FLUSH",currentdate,"");
		});//
	},(resultReject)=> {
		res.writeHead(resultReject.code, {"Content-Type": contentType_text_plain});
		res.end(resultReject.code+": Bad Request "+resultReject.text+"\n");
		var messagec = "REGISTER USER '"+ email + "' BAD REQUEST";
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, resultReject.code, req.connection.remoteAddress, messagec,currentdate,"");
	} );
});

//**********************************************************
//example:
// curl -H "Content-Type: text/plain" -XPOST http://localhost:8000/signup?name="bob"\&email="bob@abc.commm"\&pw="1234"
// app.post('/signup',ipfilter(ips, {mode: 'allow'}), function(req, res) {
app.post('/update_user', function(req, res) {
	"use strict";
	var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
	if( (req.body==undefined) && (req.query==undefined)){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Missing parameters.\n");
		return;
	}
	if(req.body==undefined) {
		req.body={};
	}
	if(req.query==undefined){
		req.query={};
	}
	var name= find_param(req.body.userid, req.query.userid);
	var email= find_param(req.body.email, req.query.email);
	var pw=find_param(req.body.pw, req.query.pw);
	if (pw == undefined){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: SIGNUP Bad Request, missing Email.\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400,req.connection.remoteAddress,"SIGNUP Bad Request, missing Email",currentdate,"");
		return;
	}else if (pw.length == 0){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: SIGNUP Bad Request, Empty Email.\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400,req.connection.remoteAddress,"SIGNUP Bad Request, Empty Email",currentdate,"");
		return;
	}
	if (email == undefined){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Bad Request, missing Email.\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400,req.connection.remoteAddress,"SIGNUP Bad Request, missing Email",currentdate,"");
		return ;
	}else if (email.length == 0){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Bad Request, Empty Email.\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400,req.connection.remoteAddress,"SIGNUP Bad Request, Empty Email",currentdate,"");
		return;
	}
	if(( req.connection.remoteAddress!= ips[0] ) &&( req.connection.remoteAddress!=ips[1])&&( req.connection.remoteAddress!=ips[2])){
		var messagea = "REGISTER USER '"+ email + "' FORBIDDEN access from external IP";
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 403,req.connection.remoteAddress,messagea,currentdate,"");
		res.writeHead(403, {"Content-Type": contentType_text_plain});
		res.end("\n403: FORBIDDEN access from external IP.\n");
		return;
	}
	var result = UsersModule.update_user(es_servername+":"+es_port,SERVERDB, name, email, pw);
	result.then((resultreg) => {
		var messageb = "UPDATE USER '"+ email + "' GRANTED";
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, resultreg.code, req.connection.remoteAddress, messageb,currentdate,"");
		var verify_flush = CommonModule.my_flush( req.connection.remoteAddress,es_servername+':'+es_port, SERVERDB);
		verify_flush.then((resolve_result) => {
			res.writeHead(resultreg.code, {"Content-Type": contentType_text_plain});
			res.end( "Succceed\n");
		},(reject_result)=> {
			res.writeHead(reject_result.code, {"Content-Type": contentType_text_plain});
			res.end(reject_result.text+"\n", 'utf-8');
		});//
	},(resultReject)=> {
		res.writeHead(resultReject.code, {"Content-Type": contentType_text_plain});
		res.end("updateuser: Bad Request "+resultReject.text+"\n");
		var messagec = "UPDATE USER '"+ email + "' BAD REQUEST";
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, resultreg.code, req.connection.remoteAddress, messagec,currentdate,"");
	});
});

//**********************************************************
//example:
// curl -H "Content-Type: text/plain" -XGET http://localhost:8000/login?email="bob"\&pw="1234" --output token.txt
app.get('/login', function(req, res) {
	"use strict";
	var resultlog;
	var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
	if( (req.body==undefined) && (req.query==undefined)){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Missing parameters.\n");
		return;
	}
	if(req.body==undefined) {
		req.body={};
	}
	if(req.query==undefined){
		req.query={};
	}
	var email= find_param(req.body.email, req.query.email);
	var pw=find_param(req.body.pw, req.query.pw);
	if (pw == undefined){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("400: Bad Request, missing Passwd\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, req.connection.remoteAddress, "400: Bad Request, missing Passwd",currentdate,"");
		return;
	}else if (pw.length == 0){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("400: Bad Request, Empty Passwd\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, req.connection.remoteAddress, "400: Bad Request, Empty Passwd",currentdate,"");
		return;
	}
	if (email == undefined){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("400: Bad Request, missing Email\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, req.connection.remoteAddress, "400: Bad Request, missing Email",currentdate,"");
		return;
	}else if (email.lenth == 0){
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("400: Bad Request, Empty Email\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, req.connection.remoteAddress, "400: Bad Request, Empty Email",currentdate,"");
		return;
	}
	var result = UsersModule.query_count_user_pw( es_servername+":"+es_port,SERVERDB, email, pw); //returns the count of email-pw, if !=1 then we consider not registered.
	result.then((resultCount) => {
		if(resultCount==1){
			var mytoken= auth.emailLogin(email);
			res.writeHead(200, {"Content-Type": contentType_text_plain});
			res.end(mytoken);
			resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 200, req.connection.remoteAddress, "New token Generated",currentdate,email);
		}else{
			res.writeHead(401, {"Content-Type": contentType_text_plain});
			res.end("401 (Unauthorized) Autentication failed, incorrect user " +" or passwd " +"\n");
// 			console.log("resultCount "+resultCount);
			resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 401, req.connection.remoteAddress,
				"401: Bad Request of Token, incorrect user \""+email+"\" or passwd or passwd ",currentdate,email);
		}
	},(resultReject)=> {
		res.writeHead(400, {"Content-Type": contentType_text_plain});
		res.end("\n400: Bad Request "+resultReject+"\n");
		resultlog = LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, req.connection.remoteAddress, 
				"400: Bad Token Request "+resultReject,currentdate,email);
	});
}); // login


app.get('/get_log_list', function(req, res) {
	"use strict";
// 	var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");  need define datetime
// 	var pretty		= find_param(req.body.pretty, req.query.pretty);
// 	var projectname	= CommonModule.remove_quotation_marks(find_param(req.body.project, req.query.project));
// 	if (projectname==undefined) projectname="";
//LogsModule
	var result_count = query_count_logs(es_servername + ":" + es_port,SERVERDB, res.user);
	result_count.then((resultResolve) => {
		if(resultResolve!=0){//new entry (2) we resister new entry
			//LogsModule
			var result_id = find_logs(es_servername + ":" + es_port,SERVERDB, res.user,"false");
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
			res.writeHead(430, {"Content-Type": contentType_text_plain});	//not reply 200 then webpage works
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
// Access to private content only if autenticated, using an authorization token
app.get('/verifytoken',middleware.ensureAuthenticated, function(req, res) {
// 	console.log("   " +colours.FgYellow + colours.Bright + " request from IP:" + req.connection.remoteAddress + colours.Reset);
	var message = "The token is valid !!!.\n"
	res.writeHead(200, { 'Content-Type': 'text/plain' });
	res.end(message, 'utf-8');
} );


//**********************************************************

app.get('/monitoringserver.html', function(req, res) {
	var filePath = 'web-monitoring/monitoringserver.html';
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

