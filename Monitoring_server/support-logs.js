//************************************************
var my_type = 'logs';

module.exports = {
	//****************************************************
//This function is used to confirm that an user exists or not in the DataBase.
query_count_logs: function(es_server, my_index, user){
	return new Promise( (resolve,reject) => {
		var elasticsearch = require('elasticsearch');
		var client = new elasticsearch.Client({
			host: es_server,
			log: 'error'
		});
		user="";
		if(user==undefined){
			user="";
		}
		if(user.length==0){
			client.count({
				index: my_index,
				type: my_type,
				body:{"query":{"match_all": {} }, "sort": { "date": { "order": "desc" }}}
			}, function(error, response) {
				if (error) {
					reject (error);
				}
				if (response.count !== undefined) {
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
				}
				if (response.count !== undefined) {
					resolve (response.count);//size
				}else{
					resolve (0);//size
				}
			});
		}
	});
}, //end query_count_project
//****************************************************
//This function is used to confirm that a project exists or not in the DataBase.
//We first counted if existence is >0
find_logs: function(es_server, my_index, user){
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
},
	//**********************************************************
	//This function is used to register log in the DB
	//example of use: 
	register_log: function(es_server,my_index,code,ip,message,date,user) { //date
		return new Promise( (resolve,reject) => {
			var myres = { code: "", text: "" };
			var elasticsearch = require('elasticsearch');
			var clientlog = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			var error="";
			var response="";
			var myres = { code: "", text:  "" };
// 			console.log(" registering log: "+user+" code : " +code+" ip: "+ip+" message: "+message+"\n");
			clientlog.index({
				index: my_index,
				type: my_type,
				body: {
					"user":user,
					"code":code,
					"ip": ip,
					"message":message,
					"date":date
					}
			}, function(error, response) {
				if (error !== 'undefined') {
					myres.code="409";
					myres.text=error;
					reject (myres);
				} else {
					myres.code="400";
					myres.text="Could not register the log.";
					reject (myres);
					return;
				}
			});//end query client.index
			myres.code="200";
			myres.text="succeed";
			resolve(myres);
		});//end promise
	}  //end register
}//end module.exports

// Example of use:
// 	var result LogsModule.register_log( 400,req.connection.remoteAddress,"MSG",currentdate,res.user);
// 	result.then((resultreg) => {
// 		....
// 	},(resultReject)=> {
// 		....
// 	} );
