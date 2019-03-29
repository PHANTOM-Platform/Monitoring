// Author: J.M.Montañana HLRS 2018
// If you find any bug, please notify to hpcjmont@hlrs.de
// 
// Copyright (C) 2018 University of Stuttgart
// 
//     Licensed under the Apache License, Version 2.0 (the "License");
//     you may not use this file except in compliance with the License.
//     You may obtain a copy of the License at
//  
//      http://www.apache.org/licenses/LICENSE-2.0
//  
//     Unless required by applicable law or agreed to in writing, software
//     distributed under the License is distributed on an "AS IS" BASIS,
//     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//     See the License for the specific language governing permissions and
//     limitations under the License.

var my_type = 'users'

module.exports = {
	//****************************************************
	//This function is used to confirm that an user exists or not in the DataBase.
	//We first counted if existence is >0
	find_user_id: function(es_server, my_index, email){ 
		return new Promise( (resolve,reject) => {
			var elasticsearch = require('elasticsearch');
			var client = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			client.search({
				index: my_index,
				type: my_type, 
				body: {
					"query":{"bool":{"must":[
							{"match_phrase":{"email": email }},
							{"term":{"email_length": email.length}}
					]}}
				}
			}, function(error, response) {
				if (error) { 
					reject (error);
				} 
				resolve (response.hits.hits[0]._id); 
			});
		});
	},
//****************************************************
	//This function is used to confirm that an user exists or not in the DataBase.
	query_count_user: function(es_server, my_index, email){ 
		return new Promise( (resolve,reject) => {
			var elasticsearch = require('elasticsearch');
			var client = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			client.count({
				index: my_index,
				type: my_type, 
				body: {
					"query":{"bool":{"must":[
							{"match_phrase":{"email":email}},
							{"term":{"email_length":email.length}}
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
		});
	},//end query_count_user
//**********************************************************
	//This function is used to verify the User and Password is registered
	query_count_user_pw: function(es_server, my_index, email,pw){
		return new Promise( (resolve,reject) => {
			var size =0;
			var elasticsearch = require('elasticsearch');
			var client = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			var count_query ={
				query: {bool:{must:[
					{match_phrase:{"email":email}},
					{term:{"email_length":email.length}},
					{match_phrase:{"password":pw}}, 
					{term:{"password_length":pw.length}}
				] } }}; 
			client.count({
				index: my_index,
				type: my_type, 
				body: count_query 
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
	},//end query_count_user_pw
//**********************************************************
	//This function is used to register new users
	//example of use: 
	register_new_user: function(es_server, my_index, name, email,pw,res) {
		return new Promise( (resolve,reject) => {
			var elasticsearch = require('elasticsearch');
			var clientb = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			var myres = { code: "", text: "" };
			var count_users = this.query_count_user(es_server, my_index, email);
			count_users.then((resultCount) => {
				if(resultCount!=0){
					var mres;
					myres.code="409";
					myres.text= "Could not register an existing user."+"\n";
					reject(myres);
				}else{
					clientb.index({
						index: my_index,
						type: my_type, 
						body: {
							"name":name,
							"email":email,
							"email_length": email.length,
							"password":pw,
							"password_length": pw.length
							}
					}, function(error, response) {
						if (error !== 'undefined') { 
							myres.code="400";
							myres.text=error;
							reject (myres); 
						} else {
							myres.code="420";
							myres.text="Could not register the user/pw." ;
							reject (myres);
						}
					});//end query client.index
					myres.code="200";
					myres.text="succeed";
					resolve(myres);
				}
			},(resultReject)=> {
					myres.code="418";
					myres.text= resultReject; 
					reject (myres);
			});//end count_users
		});//end promise
	}, //end register
//****************************************************
	//This function is used to register new users
	//example of use:
	update_user: function(es_server, my_index, name, email,pw,res) {
		return new Promise( (resolve,reject) => {
			var elasticsearch = require('elasticsearch');
			var clientb = new elasticsearch.Client({
				host: es_server,
				log: 'error'
			});
			var myres = { code: "", text: "" };
			var count_users = this.query_count_user(es_server, my_index, email);
			count_users.then((resultCount) => { 
				if(resultCount==0){
					myres.code="409";
					myres.text= "User don't found."+"\n";
					reject(myres);
				}else{
					var id_users = this.find_user_id(es_server, my_index, email);
					id_users.then((user_id) => {
						clientb.index({
							index: my_index,
							type: my_type,
							id: user_id,
							body: {
								"email":email,
								"email_length": email.length,
								"password":pw,
								"password_length": pw.length
								}
						}, function(error, response) {
							if (error !== 'undefined') { 
								myres.code="409";
								myres.text=error;
								reject (myres);
							} else {
								myres.code="409";
								myres.text="Could not register the user/pw." ;
								reject (myres);
							}
						});//end query client.index
						myres.code="200";
						myres.text="succeed";
						resolve(myres); 
					},(resultReject)=> {
						myres.code="409";
						myres.text= resultReject; 
						reject (myres);
					});//end find id_users
				}
			},(resultReject)=> { 
					myres.code="409";
					myres.text= resultReject; 
					reject (myres);
			});//end count_users
		});//end promise
	} //end register 	
}//end module.exports
