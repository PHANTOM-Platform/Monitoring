
var my_type = 'tokens'

module.exports = {
//**********************************************************
	//This function is used to register new users
	//example of use:
	register_token: function(es_server, my_index, user_id, currenttime, expirationtime) {
		return new Promise( (resolve,reject) => {
			var elasticsearch = require('elasticsearch');
			var clientb = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			var count_tokens = this.query_tokens(es_server, my_index,user_id, currenttime, expirationtime);
			count_tokens.then((resultCount) => {
				if(resultCount!=0){
					resolve ("Could not register an existing token.");
				}else{
					clientb.index({
						index: my_index,
						type: my_type,
						body: {
							"user_id":user_id,
							"currenttime": currenttime,
							"expirationtime": expirationtime
							}
					}, function(error, response) {
						if (error !== 'undefined') { 
							reject (error); 
						} else {
							reject ("Could not register the user/pw.");
						}
					});//end query client.index 
					resolve ("succeed");
				}
			},(resultReject)=> {
				reject (resultReject);
			});//end count_users
		});//end promise
	}, //end register
//****************************************************
	//This function is used to confirm that an user exists or not in the DataBase.
	query_tokens: function(es_server, my_index, user_id, currenttime, expirationtime){ 
		return new Promise( (resolve,reject) => {
			var size =0;
			var elasticsearch = require('elasticsearch');
			var client = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			client.count({
				index: my_index,
				type:  my_type, 
				body: {
					query:{bool:{must:[
						{match:{"user_id":user_id }},
						{match:{"currenttime": currenttime }},
						{match:{"expirationtime": expirationtime }} 
					]}}
				}
			}, function(error, response) {
				if (error) {
					reject (error);
				}
				if (response.count !== undefined) {
					size = response.count;
				}else{
					size=0;
				}
				resolve (size); 
			});
		});
	}//end query_user
}//end module.exports
