/**
* Copyright (c) 2018
* HLRS (Supercomputer Center Stuttgart)
*
* @summary Code for interaction of the Web Interface with the RESTful interface of the PHANTOM server
* @author Jose Miguel Montañana <montanana@hlrs.de>
* @version 1.1
* 
* Last modified : 2018-11-13 17:28:40
*/

// The defininition of server addresses may use for redirection cases, not define them if you don't know what are you doing.
// var appserver = "141.58.0.8" or "localhost";
// var appport = 8500;
// var resourceserver = "141.58.0.8" or "localhost";
// var resourceport = 2780 or 8600;
// var execserver = appserver;
// var execport = 8700;
// var reposerver = appserver;
// var repoport = 8000;
// var monitoringport = 3033

//ABOUT XHR CROSS DOMAIN REQUESTS
// Note that an XMLHttpRequest connection is subject to specific limits that are enforced for security reasons.
// One of the most obvious is the enforcement of the same origin policy.
// You cannot access resources on another server, unless the server explicitly supports this using CORS (Cross Origin Resource Sharing).

// var s = 'a string', array for [], object for {}
function getType(p) {
	if (Array.isArray(p)) return 'array';
	else if (typeof p == 'string') return 'string';
	else if (p != null && typeof p == 'object') return 'object';
	else return 'other';
}

// In case it is not defined, we define the function here.
if (!String.prototype.endsWith) {
	String.prototype.endsWith = function(searchString, position) {
		var subjectString = this.toString();
		if (typeof (position) !== 'number' || !isFinite(position) || Math.floor(position) !== position || position> subjectString.length) {
			position = subjectString.length;
		}
		position -= searchString.length;
		var lastIndex = subjectString.indexOf(searchString, position);
		return lastIndex !== -1 && lastIndex === position;
	};
}

function checktoken() {
	var menu_phantom = document.getElementById("menu_phantom");
	var menu_login = document.getElementById("menu_login");
	var phantom_operation = document.getElementById("phantom_operation");
	if(!sessionStorage.token || sessionStorage.token == undefined ) {
		if(menu_phantom) menu_phantom.style.display = "none";
		if(phantom_operation) phantom_operation.style.display = "none";
		if(menu_login) menu_login.style.display = "block";
	}else if (sessionStorage.token.length == 0) {
		if(menu_phantom) menu_phantom.style.display = "none";
		if(phantom_operation) phantom_operation.style.display = "none";
		if(menu_login) menu_login.style.display = "block";
	}else{
		if(menu_phantom) menu_phantom.style.display = "block";
		if(phantom_operation) phantom_operation.style.display = "block";
		if(menu_login) menu_login.style.display = "none";
	}
// 	if(sessionStorage.token != undefined)
// 	if(title_login) document.getElementById("title_login").innerHTML = " "+JSON.stringify(sessionStorage);
	return false;
}

function message_broadcast( ) {//requests variables from the other tags
	localStorage.setItem('getSessionStorage', 'sessionStorage.token');
	localStorage.removeItem('getSessionStorage', 'sessionStorage.token');
}

function share_session_storage_new(){
	// Ask other tabs for session storage (this is ONLY to trigger event)
	message_broadcast( );
	window.addEventListener('storage', function(event) {
		if (event.key == 'sessionStorage'){// && isEmpty(memoryStorage)) {
			sessionStorage.setItem('token', JSON.parse(event.newValue));
			checktoken();
		}
	});
	window.onbeforeunload = function() {
// 		sessionStorage.clear();
	};
	checktoken();
	return false;
}

function request_share_session_storage(){
	message_broadcast();
	localStorage.setItem('sessionStorage', JSON.stringify(sessionStorage.token));
	return false;
}

function share_session_storage_login(){
	// Ask other tabs for session storage (this is ONLY to trigger event)
	window.addEventListener('storage', function(event) {
		if (event.key == 'getSessionStorage') {
			// Some tab asked for the memoryStorage -> send it
			localStorage.clear();
			localStorage.setItem('sessionStorage', JSON.stringify(sessionStorage.token));
		}
	});
	window.onbeforeunload = function() {
// 		sessionStorage.clear();
	};
	checktoken();
	return false;
}

/**
* Stores the token in the sessionstorage, for share it among the browser tags
* @param {String} mytoken.
* @returns {boolean} true if browser supports web storage
*/
function savetoken(mytoken) {
	var debug_phantom = document.getElementById("debug_phantom");
	var demoreplaceb = document.getElementById("demoreplaceb");
	if(typeof(Storage) !== "undefined") {
// 		if (sessionStorage.token) {//update with new token
			sessionStorage.setItem('token', mytoken);
// 		}else{//not defined token before
// 			sessionStorage.setItem('token', mytoken);
// 		}
		request_share_session_storage();
		if(debug_phantom) debug_phantom.style.display = "none";
		return true;
	}else{
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, your browser does not support web storage...";
		if(debug_phantom) debug_phantom.style.display = "block";
		return false;
	}
}

function switchnightmode(){
	var body = document.getElementById("body");
	var table_results = document.getElementById("table_results");
	var currentClass = body.className;
	body.className = currentClass == "dark-mode" ? "light-mode" : "dark-mode";
		if(table_results!=null)
	table_results.className = body.className;
	localStorage.setItem('currentmode', body.className);
	var c = document.getElementById("foot_phantom").querySelectorAll("a");
	for (i in c) {
		c[i].className = body.className;
	}
	//update it at last
	document.getElementById("foot_phantom").className = body.className;
}

function start_page_login() {
	var currentmode= localStorage.getItem('currentmode');
	if(currentmode!=undefined)
		body.className = currentmode;
	share_session_storage_login();
// 	checktoken();
	return false;
}

function start_page_new() {
	var currentmode= localStorage.getItem('currentmode');
	if(currentmode!=undefined)
		body.className = currentmode;
	share_session_storage_new();
// 	checktoken();//already called at the end of share_session_storage_new
	return false;
}



function load_footer(){
	var foot_phantom = document.getElementById("foot_phantom");
	if(foot_phantom){
	var menuhtml ="";
	menuhtml+="<hr/>Web Interfaces of the PHANTOM SERVERS and MANAGERS<br>";
	if(window.location.hostname=="141.58.0.8"){
		menuhtml+="<a href=\"http://141.58.0.8:2777/repository.html\">Repository</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://141.58.0.8:2778/appmanager.html\">Application Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://141.58.0.8:2780/resourcemanager.html\">Resource Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://141.58.0.8:2781/executionmanager.html\">Execution Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://141.58.0.8:2779/monitoringserver.html\">Monitoring Server</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://141.58.0.8:2783\">Grafana Visualization Interface</a>&nbsp;&nbsp;";
	}else{
		menuhtml+="<a href=\"http://localhost:8000/repository.html\">Repository</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:8500/appmanager.html\">Application Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:8600/resourcemanager.html\">Resource Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:8700/executionmanager.html\">Execution Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:3033/monitoringserver.html\">Monitoring Server</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:3000\">Grafana Visualization Interface</a>&nbsp;&nbsp;";
	}
	menuhtml+="<hr/><div class=\"greyfont\">PHANTOM project: 2019<br/>";
	menuhtml+="	Licensed under the Apache License, Version 2.0<br/>";
	menuhtml+="	You may obtain a copy of the License at:<br/>";
	menuhtml+="	<a href=\"http://www.apache.org/licenses/LICENSE-2.0\">";
	menuhtml+="	http://www.apache.org/licenses/LICENSE-2.0</a>";
	menuhtml+="	</div>";
	foot_phantom.innerHTML = menuhtml;
	}
}



function pad(num, size) {
	var s = num+"";
	while (s.length < size) s = "0" + s;
	return s;
}


function calculate_date(input) {
	var current = input;
	current = Math.floor(current /1000000);
	var tempstr="";
	var months=[31, 28, 31, 30, 31, 30, 31, 31, 30 ,31 ,30 ,31 ];
	var exampledate_msec =Math.floor(current % 1000);
	current= Math.floor(current /1000);
	var exampledate_sec = Math.floor(current %60);
	current =Math.floor(current / 60);
	var exampledate_min = Math.floor(current % 60);
	current = Math.floor(current /60);
	var exampledate_hour = Math.floor(current %24);
	current = Math.floor(current /24);
	var exampledate_year=0;
// 	var exampledate_name_day = current % 7;
	if(current > 365+365+366){
		exampledate_year=3;
		current -= (365+365+366);
	}
	if(current > 1461){
		exampledate_year+=(4*Math.floor(current / 1461));
		current = Math.floor(current % 1461);
	}
	exampledate_year+=Math.floor(current / 365);
	current = Math.floor(current % 365);
	var leap_year;
	leap_year= (exampledate_year %4==2)? 1 : 0;
	if (leap_year==1) months[1]=29;
	var exampledate_month=0;
	while(current> months[exampledate_month]){
		current -= months[exampledate_month];
		exampledate_month++;
	}
	exampledate_year= 1970+ Math.floor(exampledate_year);
	var exampledate_day= 1+ current;
	exampledate_month=1+exampledate_month;
	tempstr=exampledate_year+"-"+pad(exampledate_month,2)+"-"+pad(exampledate_day,2)+"T"
	+pad(exampledate_hour,2)+":"+pad(exampledate_min,2)+":"+pad(exampledate_sec,2)+"."+pad(exampledate_msec,2);
	return tempstr;
}



function str2bytes (str) {
	var bytes = new Uint8Array(str.length);
	for (var i=0; i<str.length; i++) {
		bytes[i] = str.charCodeAt(i);
	}
	return bytes;
}

function ab2str(buf) {
	return String.fromCharCode.apply(null, new Uint16Array(buf));
}

function str2ab(str) {
	var buf = new ArrayBuffer(str.length*2); // 2 bytes for each char
	var bufView = new Uint16Array(buf);
	for (var i=0, strLen=str.length; i < strLen; i++) {
		bufView[i] = str.charCodeAt(i);
	}
	return buf;
}

function jsontohtml(myjson,count,first,level,lastwascoma,filtered_fields){
	var html ="";
	var i;
	if(first==true){ html ="{"; }
	var countseries=0;
	myjson.forEach(function(val) {
		if (count != 1 && lastwascoma==false) {
			if(countseries==0) {
			//exec_list removed: html += ",<br>";
			}else{
				html += "<br>},{<br>";
			}
		};//this is not the first element
		lastwascoma=true;
		var keys = Object.keys(val);
		keys.forEach(function(key) {
			if (getType(val[key]) == "string" || getType(val[key]) == "other"){//other can be a numeric value
				var tobefiltered=false;
				for (i=0;i< filtered_fields.length;i++){
					if (key.endsWith(filtered_fields[i], key.length)== true) {
						tobefiltered=true;
					}
				}
				if (tobefiltered== false) {//it is stored the length of the strings, not need to show
					if (count != 1 && lastwascoma==false) html += ',<br>';
					for (i = 0; i < level; i++) {
						if (count != 1) html += '&emsp;';
					}
					html += "<strong>\"" + key + "\"</strong>: \"" + val[key] +"\"";
					count++;
					lastwascoma=false;
				}
			}else if (getType(val[key]) == "array" ) {
					if (count != 1) html += ',<br>';
					for (i = 0; i < level; i++) {
						if (count != 1) html += '&emsp;';
					}
					html += "<strong>\"" + key + "\"</strong>: ";lastwascoma=false;
					count++;
					//exec_list replaced: html += jsontohtml( ([ val[key] ]), count, true, level+1 ,lastwascoma,filtered_fields) +"\n";
					html += jsontohtml( ([ val[key] ]), 1, true, level+1 ,lastwascoma,filtered_fields) +"\n";
			}else if (getType(val[key]) == "object" ) {
// 				html += "<tr><td><strong> &emsp;" + key + "</strong>: \"" + JSON.stringify(val[key]) +"\"</td>\n";//this shows a key counter
				//starts exec_list added
				if (count != 1) html += ',<br>';
				for (i = 0; i < level; i++) {
					if (count != 1) html += '&emsp;';
				}
				html += "<strong>\"" + key + "\"</strong>:{ ";lastwascoma=false;
				if (count != 1) html += '<br>';
				for (i = 0; i < level+1; i++) {
					if (count != 1) html += '&emsp;';
				}
				count++;
				//end exec_list added
				//exec_list replaced: html += jsontohtml( ([ val[key] ]), count, false, level+1,lastwascoma,filtered_fields) +"\n";
				html += jsontohtml( ([ val[key] ]), 1, false, level+1,lastwascoma,filtered_fields) +"\n";
			};
		});
		countseries++;
	});
	if(first==true){
		html += "<br>}";
	}
	return html;
}


function jsontotable_repo_logs_brief(typeserver, myjson,count,first,level,lastwascoma,mtitle,filtered_fields){
	var html ="";
	var i;
// 	if(first==true){ html ="{"; }
	var mainc=mtitle;
	if(first==true){
		html += "<div><table style='border:1px solid black' id=\"table_results\">\n";// style='width:100%'>";
		var typevalue=0;
		var myfunction="";
		if(typeserver==1){//repository
			typevalue=0;
			myfunction="list_repo_logs";
		}else if(typeserver==2){//app-manager
			typevalue=10;
			myfunction="list_app_logs";
		}else if(typeserver==3){// exec-manager's
			typevalue=20;
			myfunction="list_exec_logs";
		}else if(typeserver==4){//monitoring server 
			typevalue=30;
			myfunction="list_mf_logs";
		}else if(typeserver==5){// resource-manager's logs
			typevalue=40;
			myfunction="list_resource_logs";
		}
		var myvalue=201+typevalue;
		html += "<td>#</td><td align=\"center\"><a onclick=\"return "+myfunction+"("+myvalue+",document.getElementById('username').value)\" class=\"under-logs\">_id</a></td>\n";
		var myvalue=202+typevalue;
		html += "<td align=\"center\">&nbsp;<a onclick=\"return "+myfunction+"("+myvalue+",document.getElementById('username').value)\" class=\"under-logs\">Code</a>&nbsp;</td>\n";
		var myvalue=203+typevalue;
		html += "<td align=\"center\">&nbsp;<a onclick=\"return "+myfunction+"("+myvalue+",document.getElementById('username').value)\" class=\"under-logs\">User</a>&nbsp;</td>\n";
		var myvalue=204+typevalue;
		html += "<td align=\"center\">&nbsp;<a onclick=\"return "+myfunction+"("+myvalue+",document.getElementById('username').value)\" class=\"under-logs\">Ip</a>&nbsp;</td>\n";
		var myvalue=205+typevalue;
		html += "<td align=\"center\">&nbsp;<a onclick=\"return "+myfunction+"("+myvalue+",document.getElementById('username').value)\" class=\"under-logs\">Message</a>&nbsp;</td>\n";
		var myvalue=200+typevalue;
		html += "<td align=\"center\">&nbsp;<a onclick=\"return "+myfunction+"("+myvalue+",document.getElementById('username').value)\" class=\"under-logs\">Date</a>&nbsp;</td>\n";
		count++;
	}
	first=false;
	var countseries=0;
	myjson.forEach(function(val) {
// 		if (count != 0 && lastwascoma==false) {
// 			html += (countseries==0) ? ",<br>" : "<br>},{<br>";
// 		};//this is not the first element
		lastwascoma=true;
		var keys = Object.keys(val);
		keys.forEach(function(key) {
			if (getType(val[key]) == "string" || getType(val[key]) == "other" ){
				var tobefiltered=false;
				for (i=0;i< filtered_fields.length;i++){
					if (key.endsWith(filtered_fields[i], key.length)== true) {
						tobefiltered=true;
					}
				}
				if (tobefiltered== false) {//it is stored the length of the strings, not need to show
// 					if (count != 0 && lastwascoma==false) html += ',<br>';
// 					for (i = 0; i < level; i++) {
// 						if (count != 0) html += '&emsp;';
// 					}
					if(mtitle==true){
						if(count>0){
							html += "</tr>\n<tr>";
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<td>"+count+"</td><th>" + val['_id'] +"</th>\n";
						//source
						if(val['_source'] !=undefined){
							if(val['_source']['code']==undefined){
								html += "<td>";
							}else if((val['_source']['code']>="200")&&(val['_source']['code']<"300")){//green 2xx-correct,3xx-redirections
								html += "<td bgcolor=\"#00ff00\"> <font color=\"black\">" + val['_source']['code'] +"</font>";
							}else if((val['_source']['code']>="400") && (val['_source']['code']<"600")) {//red 4xx-client-error 5xx-server-error
								html += "<td bgcolor=\"#ff3e29\"> <font color=\"black\">" + val['_source']['code'] +"</font>";
							}else if((val['_source']['code']>="100")&&(val['_source']['code']<"200")){ //yellow-information
								html += "<td bgcolor=\"#f3ff3a\"> <font color=\"black\">" + val['_source']['code'] +"</font>";
	// 						}else if(val['_source']['code']=="cancelled"){//red
	// 							html += "<td bgcolor=\"#ff3e29\"> " + val['_source']['code'];
	// 						}else if(val['_source']['code']=="started"){//green
	// 							html += "<td bgcolor=\"#00FF00/*\*/">" + val['_source']['code'];
	// 						}else{
	// // 						html += "<td> " + val['_source']['code'];
							}
							html += "</td>\n<td>"; html += (val['_source']['user']==undefined)? "" : val['_source']['user'];
							html += "</td>\n<td>"; html += (val['_source']['ip']==undefined)? "" : val['_source']['ip'];
							html += "</td>\n<td>"; html += (val['_source']['message']==undefined)? "" : val['_source']['message'];
							html += "</td>\n<td>"; html += (val['_source']['date']==undefined)? "" : val['_source']['date'];
							html += "</td>\n";
						}else{
							html += "<td></td>\n";
							html += "<td></td>\n";
							html += "<td></td>\n";
							html += "<td></td>\n";
							html += "<td></td>\n";
						}
						mtitle=false;
						lastwascoma=false;
					}
// 					if((key=="rejection_reason")){
// 						if(val['req_status']=="rejected"){
// 							html += "<td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td>\n";
// 							count++;
// 							lastwascoma=false;
// 						}
// 					}else if((key!="req_status")&&(key!="energy")&&(key!="execution_id")&&(key!="app")&&(key!="device")){
// 						html += "<td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td>\n";
// 						count++;
// 						lastwascoma=false;
				}
			}else if (getType(val[key]) == "array" || getType(val[key]) == "object" ) {
// 				if(key!= "component_stats"){
// // 					if (count != 0) html += ',<br>';
// // 					for (i = 0; i < level; i++) {
// // 						if (count != 0) html += '&emsp;';
// // 					}
// 					if(mtitle==true){
// 						if(count>0){
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
// 						}
// 						html += "<tr><th><strong>\"" + key + "\"</strong>:</th>\n";
// 						mtitle=false;
// 					}else{
// 						html += "<tr><td><strong>\"" + key + "\"</strong>:</td>\n";
// 					}
// 					count++;
// 					lastwascoma=false;
// 					html += "<td><div><table style='width:100%; border:0px solid black'>\n";// style='width:100%'>";
					html += jsontotable_repo_logs_brief(typeserver, ([ val[key] ]), count, first, level+1 ,lastwascoma,mtitle,filtered_fields);
					count++;
// 					html += "</table></div></td>\n";
// 				}
// // 			}else if (getType(val[key]) == "object" ) {
// // 				html += jsontotable( ([ val[key] ]), count, false, level+1,lastwascoma,mtitle,filtered_fields);
			};
		});
// 		mtitle=true;
		countseries++;
	});
// 	if(first==true){ html += "<br>}"; }
// 	if(mainc==true)
// 		html += "</table></div>\n";
	return html;
}//jsontotable_repo_logs_brief


function upload_with_token( UploadJSON, url ) {
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
// 	share_session_storage();
	if(!sessionStorage.token) {
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
		return false;
	}
	if((sessionStorage.token !== undefined) && (sessionStorage.token.length>0)) {
		var xhr = new XMLHttpRequest();
		var formData = new FormData();
		xhr.open('POST', url, true);
		xhr.setRequestHeader("Authorization", "JWT " + sessionStorage.token);
		xhr.addEventListener('load', function() {
			var responseObject = (xhr.responseText);
			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>"+ responseObject + " status: " +xhr.status+ "</pre>";
			if(debug_phantom) debug_phantom.style.display = "block";
		});
		formData.append("UploadJSON", UploadJSON.files[0]);
//formData.append("UploadFile", UploadFile.data);
		xhr.send(formData);//may fault code appear here
	}else{
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
	}
	return false;
}

function list_results_with_token( mytype ,url,fields_toshow, filtered_fields) {
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	if((sessionStorage.token) && (sessionStorage.token.length>0)) {//reject null, undefined and empty string
		list_results(mytype,url,fields_toshow,filtered_fields);
	}else{
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
	}
	return false;
}

//_filter_workflow_taskid_experimentid
function jsontotable(myjson,count,first,level,lastwascoma,mtitle,filtered_fields){
	var html ="";
	var i;
// 	if(first==true){ html ="{"; }
	var mainc=mtitle;
	if(mtitle==true){
		html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
	}
	var countseries=0;
	myjson.forEach(function(val) {
// 		if (count != 1 && lastwascoma==false) {
// 			html += (countseries==0) ? ",<br>" : "<br>},{<br>";
// 		};//this is not the first element
		lastwascoma=true;
		var keys = Object.keys(val);
		keys.forEach(function(key) {
			if (getType(val[key]) == "string" || getType(val[key]) == "other" ){
				var tobefiltered=false;
				for (i=0;i< filtered_fields.length;i++){
					if (key.endsWith(filtered_fields[i], key.length)== true) {
						tobefiltered=true;
					}
				}
				if (tobefiltered== false) {//it is stored the length of the strings, not need to show
// 					if (count != 1 && lastwascoma==false) html += ',<br>';
// 					for (i = 0; i < level; i++) {
// 						if (count != 1) html += '&emsp;';
// 					}
					if(mtitle==true){
						if(count>1){
							html += "</table></div></td><br>\n";
							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<tr><th><strong>\""+ key +"\"</strong>: \"" + val[key] +"\"</th></tr>\n";
					}else{
						html += "<tr><td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td></tr>\n";
					}
					mtitle=false;
					count++;
					lastwascoma=false;
				}
			}else if (getType(val[key]) == "array" || getType(val[key]) == "object" ) {
// 					if (count != 1) html += ',<br>';
// 					if (count != 1) for (i = 0; i < level; i++) html += '&emsp;';
					if(mtitle==true){
						if(count>1){
							html += "</table></div></td><br>\n";
							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<tr><th><strong>\"" + key + "\"</strong>:</th>\n";
					}else{
						html += "<tr><td><strong>\"" + key + "\"</strong>:</td>\n";
					}
					mtitle=false;
					count++;
					lastwascoma=false;
					html += "<td><div><table style='width:100%; border:0px solid black'>\n";// style='width:100%'>";
					html += jsontotable( ([ val[key] ]), count, true, level+1 ,lastwascoma,mtitle,filtered_fields);
					html += "</table></div></td>\n";
// 			}else if (getType(val[key]) == "object" ) {
// 				html += jsontotable( ([ val[key] ]), count, false, level+1,lastwascoma,mtitle,filtered_fields);
			};
		});
		mtitle=true;
		countseries++;
	});
// 	if(first==true) html += "<br>}";
	if(mainc==true)
		html += "</table></div>\n";
	return html;
}//jsontotable


function list_results(mytype, url,fields_toshow,filtered_fields){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var xhr = new XMLHttpRequest();
	xhr.open("GET", url, true);
	if(sessionStorage.token !== undefined){
		if(sessionStorage.token.length>0) {
			xhr.setRequestHeader("Authorization", "JWT " + sessionStorage.token);
	}}
	xhr.addEventListener('load', function() {
// 	xhr.onreadystatechange = function() {
		var html = "";// to store the conversion of json to html format
		if (xhr.readyState === 4 && xhr.status == 200) {
// 			var responseObject = xhr.responseText;
			var responseObject = [ xhr.responseText ];
			// document.getElementById('demoreplacea').innerHTML = responseObject;//this will show the reponse of the server as txt;
			var myjson = JSON.parse(responseObject || '{}');
			if(myjson.hits!=undefined) {
// 				console.log("myjsob "+JSON.stringify(myjson));
				myjson = myjson.hits;
			}else{
				myjson = [ myjson ];
			}
			if(myjson!=undefined) {
				if (mytype== 1) {
					html += jsontotable(myjson,1,true,1,false,true,filtered_fields);
				}else if ( (mytype >= 200) && (mytype < 207 )){//repository
					html += jsontotable_repo_logs_brief(1,myjson,0,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if ( (mytype >= 210) && (mytype < 217 )){//app manager
					html += jsontotable_repo_logs_brief(2,myjson,0,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if ( (mytype >= 220) && (mytype < 227 )){ // exec-manager's logs, 8XX for sorting
					html += jsontotable_repo_logs_brief(3,myjson,0,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if ( (mytype >= 230) && (mytype < 237 )){//monitoring server
					html += jsontotable_repo_logs_brief(4,myjson,0,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if ( (mytype >= 240) && (mytype < 247 )){// resource-manager's logs
					html += jsontotable_repo_logs_brief(5,myjson,0,true,1,false,true,filtered_fields);
					html += "</table></div>\n";

				}else if ( (mytype >= 250) && (mytype < 257 )){//resource manager 
					html += jsontotable_rm_brief(myjson,1,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
					
				}else if ((mytype == 5) || ((mytype > 800) && (mytype < 812 ))){ //8XX for sorting 
					html += jsontotable_exec_brief(myjson,1,true,1,false,true,filtered_fields);
				}else if ((mytype == 6) || ((mytype > 900) && (mytype < 907 ))){//9XX for sorting
					html += jsontotable_app_brief(myjson,1,true,1,false,true,filtered_fields);
				}else if (mytype == 4){
					html += jsontotable_only_device_names(myjson,1,true,1,false,true,fields_toshow);
				}else if (mytype == 2){
					html += jsontohtml(myjson,1,true,1,false,filtered_fields);
				}else{
					html += "<p align=\"justify\">"+ JSON.stringify(myjson)+ "</p>";
				}
			}
			if (demoreplaceb) demoreplaceb.innerHTML = html; //+responseObject + " status: " +xhr.status;
			if (debug_phantom) debug_phantom.style.display = "block";
			//demoreplaceb.innerHTML = JSON.stringify(myjson) + "<br>" + html;// myjson[0].project;
		}
	});
	xhr.send(null);
	return false;
}



function download_file(content, fileName, contentType) {
// 	var file = new Blob([str2bytes(content)], {type: contentType});
	var file = new Blob([content], {type: contentType});
// 	if (navigator.msSaveOrOpenBlob) {
// 		navigator.msSaveOrOpenBlob(file, filename);
// 	}else{
		var a = document.createElement("a");
		document.body.appendChild(a);
		var url = URL.createObjectURL(file);
		a.style = "display:none";
		a.href = url;
		a.download = fileName;
		a.click();
// 		URL.revokeObjectURL(url);
		a.remove();
	// 	saveAs(file, fileName);
// 	}
}


/**
* @return a file with the server response if a outputfilename is provided
* */
function submitform(url, operation, outputfile) {
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var phantom_operation = document.getElementById("phantom_operation");
	if((sessionStorage.token !== undefined) && (sessionStorage.token.length>0)) {
		var xhr = new XMLHttpRequest();
		xhr.open(operation, url, true);
		xhr.setRequestHeader("Authorization", "JWT " + sessionStorage.token);
		xhr.setRequestHeader('X-Requested-With','XMLHttpRequest');
//		xhr.setRequestHeader('Content-Type', 'application/json; charset=UTF-8'); mal
		xhr.addEventListener('load', function() {
			if (xhr.readyState == 4) {
				var r = "";
				if (xhr.status == 200 || xhr.status == 0) {
					if(outputfile.length>0)
						download_file(xhr.responseText, outputfile , 'text/plain');
					r = "Server response:<br/><br/>"+xhr.responseText+"<br/>";
				}else{
					r = "Error " + xhr.status + " occurred requesting for Metatada.<br/>";
				}
				if(demoreplaceb) document.getElementById("demoreplaceb").innerHTML = "<pre>"+r+"</pre>";
				if(debug_phantom) document.getElementById("debug_phantom").style.display = "block";
				if(phantom_operation) document.getElementById("phantom_operation").style.display = "none";
			}
		});
		xhr.send(null);//may fault code appear here
	}else{
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
	}
	return false;
}
