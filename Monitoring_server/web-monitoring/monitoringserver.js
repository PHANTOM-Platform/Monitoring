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

/**
* Returns the host and port (if defined) for building the url
* @returns {String} beginning of the url
*/
function build_resource_path(){
	var url="";
	if(typeof (resourceserver)!== 'undefined'){ // Any scope
		if(resourceserver){
		if(resourceserver.length>0){
			url=url+"http://"+resourceserver;
			if(typeof resourceport!== 'undefined') {// Any scope
				if ((resourceport) && resourceport.lenght>0){
					url=url+":"+resourceport;
	}	}}	}	}
	return url;
}

function build_monitoring_path(){
	var url="";
	if(typeof (monitoringserver)!== 'undefined'){ // Any scope
		if(monitoringserver){
		if(monitoringserver.length>0){
			url=url+"http://"+monitoringserver;
			if(typeof monitoringport!== 'undefined') {// Any scope
				if ((monitoringport) && monitoringport.lenght>0){
					url=url+":"+monitoringport;
	}	}}	}	}
	return url;
}

function build_appman_path(){
	var url="";
	if(typeof appserver!== 'undefined'){ // Any scope
		if(appserver){
		if(appserver.length>0){
			url=url+"http://"+appserver;
			if(typeof appport!== 'undefined') {// Any scope
				if ((appport) && appport.lenght>0){
					url=url+":"+appport;
	}	}	}	}}
	return url;
}

function build_execman_path(){
	var url="";
	if(typeof execserver!== 'undefined'){ // Any scope
		if(execserver){
		if(execserver.length>0){
			url=url+"http://"+execserver;
			if(typeof execport!== 'undefined') {// Any scope
				if ((execport) && execport.lenght>0){
					url=url+":"+execport;
	}	}}	}	}
	return url;
}

function build_repo_path(){
	var url="";
	if(typeof reposerver!== 'undefined'){ // Any scope
		if(reposerver){
		if(reposerver.length>0){
			url=url+"http://"+reposerver;
			if(typeof repoport!== 'undefined') {// Any scope
				if ((repoport) && repoport.lenght>0){
					url=url+":"+repoport;
	}	}	}	}}
	return url;
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
		if (sessionStorage.token) {//update with new token
			sessionStorage.setItem('token', mytoken);
		}else {//not defined token before
			sessionStorage.setItem('token', mytoken);
		}
		request_share_session_storage();
		if(debug_phantom) debug_phantom.style.display = "none";
		return true;
	}else {
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, your browser does not support web storage...";
		if(debug_phantom) debug_phantom.style.display = "block";
		return false;
	}
}

// var currentbody = "light-mode";

function switchnightmode(){
	var body = document.getElementById("body");
	var table_results = document.getElementById("table_results");
	var currentClass = body.className;
	body.className = currentClass == "dark-mode" ? "light-mode" : "dark-mode";
	table_results.className = body.className ;
	localStorage.setItem('currentmode',  body.className);
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

function mf_logout() {
	sessionStorage.setItem('token', '');
	request_share_session_storage();
// 	checktoken(); //already called at the end of request_share_session_storage
	window.location = 'monitoringserver.html';
	return false;
}

function rm_logout() {
	sessionStorage.setItem('token', '');
	request_share_session_storage();
// 	checktoken();//already called at the end of request_share_session_storage
	window.location = 'resourcemanager.html';
	return false;
}

function app_logout() {
	sessionStorage.setItem('token', '');
	request_share_session_storage();
// 	checktoken();//already called at the end of request_share_session_storage
	window.location = 'appmanager.html';
	return false;
}

function repo_logout() {
	sessionStorage.setItem('token', '');
	request_share_session_storage();
// 	checktoken();//already called at the end of request_share_session_storage
	window.location = 'repository.html';
	return false;
}

function exec_logout() {
	sessionStorage.setItem('token', '');
	request_share_session_storage();
// 	checktoken();//already called at the end of request_share_session_storage
	window.location = 'executionmanager.html';
	return false;
}

/**
* Loads the phantom login menu into the html element "meny_login"
* @returns {Boolean} return true if succeed.
*/
function rm_load_menu_login(){
	var menu_login = document.getElementById("menu_login");
	if(menu_login){
	var menuhtml="<H1 id=\"title_login\" style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>LOGIN into RESOURCE-MANAGER</b></H1>";
	menuhtml+="<form";
	menuhtml+="	id='requestToken'";
	menuhtml+="	method='get'";
	menuhtml+="	name=\"myForm\" autocomplete=\"on\">";
// <!-- 		encType="multipart/form-data"> //for post not for get-->
	menuhtml+="	<div class=\"center\">";
	menuhtml+="		User: <input type=\"text\" name=\"user\" id=\"user\" value=\"\"><br>";
	menuhtml+="		Password: <input type=\"password\" name=\"password\" id=\"password\" value=\"\" autocomplete=\"off\"> <br>";
	menuhtml+="		<input type=\"hidden\" name=\"pretty\" value=\"true\" />";
	menuhtml+="		<input type=\"submit\" onclick=\" rm_login(document.getElementById('user').value, document.getElementById('password').value); return false;\" value=\"LOGIN\" />";
	menuhtml+="	</div>";
	menuhtml+="</form>";
	menu_login.innerHTML = menuhtml;
	return true;
	}else{
		return false;
	}
}

// function mf_load_menu_login(){
// 	var menu_login = document.getElementById("menu_login");
// 	if(menu_login){
// 		var menuhtml="<H1 id=\"title_login\" style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>LOGIN into MONITORING-SERVER</b></H1>";
// 		menuhtml+="<form";
// 		menuhtml+="	id='requestToken'";
// 		menuhtml+="	method='get'";
// 		menuhtml+="	name=\"myForm\" autocomplete=\"on\">";
// 	// <!-- 		encType="multipart/form-data"> //for post not for get-->
// 		menuhtml+="	<div class=\"center\">";
// 		menuhtml+="		User: <input type=\"text\" name=\"user\" id=\"user\" value=\"\"><br>";
// 		menuhtml+="		Password: <input type=\"password\" name=\"password\" id=\"password\" value=\"\" autocomplete=\"off\"> <br>";
// 		menuhtml+="		<input type=\"hidden\" name=\"pretty\" value=\"true\" />";
// 		menuhtml+="		<input type=\"submit\" onclick=\" mf_login(document.getElementById('user').value, document.getElementById('password').value); return false;\" value=\"LOGIN\" />";
// 		menuhtml+="	</div>";
// 		menuhtml+="</form>";
// 		menu_login.innerHTML = menuhtml;
// 		return true;
// 	}else{
// 		return false;
// 	}
// }

function repo_load_menu_login(){
	var menu_login = document.getElementById("menu_login");
	if(menu_login){
	var menuhtml="<H1 id=\"title_login\" style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>LOGIN into REPOSITORY-Server</b></H1>";
	menuhtml+="<form";
	menuhtml+="	id='requestToken'";
	menuhtml+="	method='get'";
	menuhtml+="	name=\"myForm\" autocomplete=\"on\">";
// <!-- 		encType="multipart/form-data"> //for post not for get-->
	menuhtml+="	<div class=\"center\">";
	menuhtml+="		User: <input type=\"text\" name=\"user\" id=\"user\" value=\"\"><br>";
	menuhtml+="		Password: <input type=\"password\" name=\"password\" id=\"password\" value=\"\" autocomplete=\"off\"> <br>";
	menuhtml+="		<input type=\"hidden\" name=\"pretty\" value=\"true\" />";
	menuhtml+="		<input type=\"submit\" onclick=\" repo_login(document.getElementById('user').value, document.getElementById('password').value); return false;\" value=\"LOGIN\" />";
	menuhtml+="	</div>";
	menuhtml+="</form>";
	menu_login.innerHTML = menuhtml;
	return true;
	}else{
		return false;
	}
}

function app_load_menu_login(){
	var menu_login = document.getElementById("menu_login");
	if(menu_login){
	var menuhtml="<H1 id=\"title_login\" style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>LOGIN into APP-MANAGER</b></H1>";
	menuhtml+="<form";
	menuhtml+="	id='requestToken'";
	menuhtml+="	method='get'";
	menuhtml+="	name=\"myForm\" autocomplete=\"on\">";
// <!-- 		encType="multipart/form-data"> //for post not for get-->
	menuhtml+="	<div class=\"center\">";
	menuhtml+="		User: <input type=\"text\" name=\"user\" id=\"user\" value=\"\"><br>";
	menuhtml+="		Password: <input type=\"password\" name=\"password\" id=\"password\" value=\"\" autocomplete=\"off\"> <br>";
	menuhtml+="		<input type=\"hidden\" name=\"pretty\" value=\"true\" />";
	menuhtml+="		<input type=\"submit\" onclick=\" applogin(document.getElementById('user').value, document.getElementById('password').value); return false;\" value=\"LOGIN\" />";
	menuhtml+="	</div>";
	menuhtml+="</form>";
	menu_login.innerHTML = menuhtml;
	return true;
	}else{
		return false;
	}
}

function exec_load_menu_login(){
	var menu_login = document.getElementById("menu_login");
	if(menu_login){
	var menuhtml="<H1 id=\"title_login\" style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>LOGIN into EXECUTION-MANAGER</b></H1>";
	menuhtml+="<form";
	menuhtml+="	id='requestToken'";
	menuhtml+="	method='get'";
	menuhtml+="	name=\"myForm\" autocomplete=\"on\">";
// <!-- 		encType="multipart/form-data"> //for post not for get-->
	menuhtml+="	<div class=\"center\">";
	menuhtml+="		User: <input type=\"text\" name=\"user\" id=\"user\" value=\"\"><br>";
	menuhtml+="		Password: <input type=\"password\" name=\"password\" id=\"password\" value=\"\" autocomplete=\"off\"> <br>";
	menuhtml+="		<input type=\"hidden\" name=\"pretty\" value=\"true\" />";
	menuhtml+="		<input type=\"submit\" onclick=\" exec_login(document.getElementById('user').value, document.getElementById('password').value); return false;\" value=\"LOGIN\" />";
	menuhtml+="	</div>";
	menuhtml+="</form>";
	menu_login.innerHTML = menuhtml;
	return true;
	}else{
		return false;
	}
}

function rm_load_header(){
	var menu_phantom = document.getElementById("menu_phantom");
	if(menu_phantom){
	var menuhtml="<ul class=\"menuphantom\">";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_list.html\">List of registered DEVICEs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_new.html\">Register new DEVICE with a JSON file</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_update.html\">Update a DEVICE with a JSON file</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_update_form.html\">Register a DEVICE with a Form</a></li>";
// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_update1.json\">Download JSON example</a></li>";
// <!-- <li class="menuphantom"><a class="active" href="download_file.html">Download File</a></li> -->
	menuhtml+="	<li class=\"phantomlogo\" style=\"float:right\">";
	menuhtml+="	<img src=\"phantom.gif\" alt=\"PHANTOM\" height=\"32\" style=\"background-color:white;\"></img>";
	menuhtml+="	</li>";
	menuhtml+="	<li class=\"menuphantomR\">";
	menuhtml+="		<p><a onClick=\"rm_logout();return false;\" href=\"PleaseEnableJavascript.html\">LogOut</a></p></li>";
	menuhtml+="</ul>";
	menuhtml+="<ul class=\"menuphantom\">";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_mf_config_list.html\">List the MF config of the registered DEVICEs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_mf_config_reg.html\">Register/Update an MF-Configuration with a JSON file</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_mf_config_form.html\">Register an MF configuration with a Form</a></li>";
	menuhtml+="</ul>";
	menuhtml+="<ul class=\"menuphantom\">";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"device_status_list.html\">Current load of the registered DEVICEs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"log_list.html\">List of logs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><input type=\"button\" value=\"Night mode\" onclick=\"switchnightmode()\"></a></li>";
	menuhtml+="</ul>";
	menu_phantom.innerHTML = menuhtml;
	}
}

function mf_load_header(){
	var menu_phantom = document.getElementById("menu_phantom");
	if(menu_phantom){
	var menuhtml="<ul class=\"menuphantom\">";
// 	menuhtml+="	<li class=\"menuphantom\"><font color=\"white\">here go the options</font></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"log_list.html\">List of logs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><input type=\"button\" value=\"Night mode\" onclick=\"switchnightmode()\"></a></li>";

// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_list.html\">List of registered APPs</a></li>";
// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_new.html\">Register new APP</a></li>";
// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update.html\">Update an APP</a></li>";
// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update1.json\">Download JSON example 1</a></li>";
// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update2.json\">Download JSON example 2</a></li>";
// 	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update3.json\">Download JSON example 3</a></li>";
// <!--<li class="menuphantom"><a href="query_metadata.html">Query metadata</a></li> -->
	menuhtml+="	<li class=\"phantomlogo\" style=\"float:right\">";
	menuhtml+="	<img src=\"phantom.gif\" alt=\"PHANTOM\" height=\"32\" style=\"background-color:white;\">";
	menuhtml+="	</li>";
// 	menuhtml+="	<li class=\"menuphantomR\">";
// 	menuhtml+="		<p><a onClick=\"mf_logout();return false;\" href=\"PleaseEnableJavascript.html\">LogOut</a></p></li>";
	menuhtml+="</ul>";
	menu_phantom.innerHTML = menuhtml;
	}
}



function app_load_header(){
	var menu_phantom = document.getElementById("menu_phantom");
	if(menu_phantom){
	var menuhtml="<ul class=\"menuphantom\">";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_list.html\">List of registered APPs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_new.html\">Register new APP</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update.html\">Update an APP</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update1.json\">Download JSON example 1</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update2.json\">Download JSON example 2</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"app_update3.json\">Download JSON example 3</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"log_list.html\">List of logs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><input type=\"button\" value=\"Night mode\" onclick=\"switchnightmode()\"></a></li>";
// <!--<li class="menuphantom"><a href="query_metadata.html">Query metadata</a></li> -->
	menuhtml+="	<li class=\"phantomlogo\" style=\"float:right\">";
	menuhtml+="	<img src=\"phantom.gif\" alt=\"PHANTOM\" height=\"32\" style=\"background-color:white;\">";
	menuhtml+="	</li>";
	menuhtml+="	<li class=\"menuphantomR\">";
	menuhtml+="		<p><a onClick=\"app_logout();return false;\" href=\"PleaseEnableJavascript.html\">LogOut</a></p></li>";
	menuhtml+="</ul>";
	menu_phantom.innerHTML = menuhtml;
	}
}

function exec_load_header(){
	var menu_phantom = document.getElementById("menu_phantom");
	if(menu_phantom){
	var menuhtml="<ul class=\"menuphantom\">";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"exec_list.html\">List of executed APPs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"exec_new.html\">Register new Execution</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"exec_update.html\">Update an Execution</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"exec_update1.json\">Download JSON example 1</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"exec_update2.json\">Download JSON example 2</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"log_list.html\">List of logs</a></li>";	
	menuhtml+="	<li class=\"menuphantom\"><input type=\"button\" value=\"Night mode\" onclick=\"switchnightmode()\"></a></li>";
// <!--<li class="menuphantom"><a href="query_metadata.html">Query metadata</a></li> -->
	menuhtml+="	<li class=\"phantomlogo\" style=\"float:right\">";
	menuhtml+="	<img src=\"phantom.gif\" alt=\"PHANTOM\" height=\"32\" style=\"background-color:white;\">";
	menuhtml+="	</li>";
	menuhtml+="	<li class=\"menuphantomR\">";
	menuhtml+="		<p><a onClick=\"app_logout();return false;\" href=\"PleaseEnableJavascript.html\">LogOut</a></p></li>";
	menuhtml+="</ul>";
	menu_phantom.innerHTML = menuhtml;
	}
}

function repo_load_header(){
	var menu_phantom = document.getElementById("menu_phantom");
	if(menu_phantom){
	var menuhtml="<ul class=\"menuphantom\">";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"query_metadata.html\">Query metadata</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"file_list.html\">List files</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"upload_file.html\">Register new file</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"download_file.html\">Download a file</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"download_zip.html\">Download a zip file</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"examplec.json\">Download JSON example</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><a href=\"log_list.html\">List of logs</a></li>";
	menuhtml+="	<li class=\"menuphantom\"><input type=\"button\" value=\"Night mode\" onclick=\"switchnightmode()\"></a></li>";

// 	class="active"
	menuhtml+="	<li class=\"phantomlogo\" style=\"float:right\">";
	menuhtml+="	<img src=\"phantom.gif\" alt=\"PHANTOM\" height=\"32\" style=\"background-color:white;\">";
	menuhtml+="	</li>";
	menuhtml+="	<li class=\"menuphantomR\">";
	menuhtml+="		<p><a onClick=\"repo_logout();return false;\" href=\"PleaseEnableJavascript.html\">LogOut</a></p></li>";
	menuhtml+="</ul>";
	menu_phantom.innerHTML = menuhtml;
	}
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
		menuhtml+="<a href=\"http://141.58.0.8:2779/monitoringserver.html\">Monitoring Server</a>";
	}else{
		menuhtml+="<a href=\"http://localhost:8000/repository.html\">Repository</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:8500/appmanager.html\">Application Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:8600/resourcemanager.html\">Resource Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:8700/executionmanager.html\">Execution Manager</a>&nbsp;&nbsp;";
		menuhtml+="<a href=\"http://localhost:3033/monitoringserver.html\">Monitoring Server</a>";
	}
	menuhtml+="<hr/><div class=\"greyfont\">PHANTOM project: 2019<br />";
	menuhtml+="	Licensed under the Apache License, Version 2.0<br />";
	menuhtml+="	You may obtain a copy of the License at:<br />";
	menuhtml+="	<a href=\"http://www.apache.org/licenses/LICENSE-2.0\">";
	menuhtml+="	http://www.apache.org/licenses/LICENSE-2.0</a>";
	menuhtml+="	</div>";
	foot_phantom.innerHTML = menuhtml;
	}
}

function repo_load_header_footer(){
	repo_load_header();
	repo_load_menu_login();
	load_footer();
	checktoken();
}

function rm_load_header_footer(){
	rm_load_header();
	rm_load_menu_login();
	load_footer();
	checktoken();
}

function mf_load_header_footer(){
	mf_load_header();
// 	mf_load_menu_login(); //not implemented login in the mf server
	mf_login("dummy","dummy");
	load_footer();
// 	checktoken();
}

function app_load_header_footer(){
	app_load_header();
	app_load_menu_login();
	load_footer();
	checktoken();
}

function exec_load_header_footer(){
	exec_load_header();
	exec_load_menu_login();
	load_footer();
	checktoken();
}

function applogin(user,password){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var url = build_appman_path() + "/login?email="+user+"\&pw="+password+"";//?pretty='true'";
	var xhr = new XMLHttpRequest();
	xhr.open("GET", url, true);
	xhr.onreadystatechange = function() {
		if (xhr.status == 200) {
			var serverResponse = xhr.responseText;
			savetoken(serverResponse);
			checktoken();
		}else{
			var serverResponse = xhr.responseText;
			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>Error: "+ serverResponse+ "</pre>";
			console.log("Error: "+ serverResponse);
			if(debug_phantom) debug_phantom.style.display = "block";
			app_logout();
			checktoken();
		}
	};
	xhr.send(null);
	return false;
}

function rm_login(user,password){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var menu_login = document.getElementById("menu_login");
	var menu_phantom = document.getElementById("menu_phantom"); //top menu
	var url=build_resource_path()+"/login?email="+user+"\&pw="+password+"";//?pretty='true'";
	var xhr = new XMLHttpRequest();
	xhr.open("GET", url, true);
	xhr.onreadystatechange = function() {
		if (xhr.status == 200) {
			var serverResponse = xhr.responseText;
			savetoken(serverResponse);
			checktoken();
			var menuhtml="<H1 style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>Choose one option from the top menu</b></H1>";
			if(menu_login) menu_login.innerHTML = menuhtml;
			if(menu_login) menu_login.style.display = "block";
		}else{
			rm_logout();
// 			checktoken();
			rm_load_menu_login();
			if(menu_login) menu_login.style.display = "block";
			var serverResponse = xhr.responseText;
			if(menu_phantom) menu_phantom.style.display = "none";
			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>Error: "+ serverResponse+ "</pre>";
			if(debug_phantom) debug_phantom.style.display = "block";
		}
	};
	xhr.send(null);
	return false;
}


function mf_login(user,password){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var menu_login = document.getElementById("menu_login");
	var menu_phantom = document.getElementById("menu_phantom"); //top menu
	
//not implemented login in the MF_server
	
// 	var url=build_resource_path()+"/login?email="+user+"\&pw="+password+"";//?pretty='true'";
// 	var xhr = new XMLHttpRequest();
// 	xhr.open("GET", url, true);
// 	xhr.onreadystatechange = function() {
// 		if (xhr.status == 200) {
// 			var serverResponse = xhr.responseText;
// 			savetoken(serverResponse);
// 			checktoken();
			var menuhtml="<H1 style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>Choose one option from the top menu</b></H1>";
			if(menu_login) menu_login.innerHTML = menuhtml;
			if(menu_login) menu_login.style.display = "block";
// 		}else{
// 			mf_logout();
// // 			checktoken();
// 			mf_load_menu_login();
// 			if(menu_login) menu_login.style.display = "block";
// 			var serverResponse = xhr.responseText;
// 			if(menu_phantom) menu_phantom.style.display = "none";
// 			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>Error: "+ serverResponse+ "</pre>";
// 			if(debug_phantom) debug_phantom.style.display = "block";
// 		}
// 	};
// 	xhr.send(null);
	return false;
}

function repo_login(user,password){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var menu_login = document.getElementById("menu_login");
	var menu_phantom = document.getElementById("menu_phantom"); //top menu
	var url=build_repo_path()+"/login?email="+user+"\&pw="+password+"";//?pretty='true'";
	var xhr = new XMLHttpRequest();
	xhr.open("GET", url, true);
	xhr.onreadystatechange = function() {
		if (xhr.status == 200) {
			var serverResponse = xhr.responseText;
			savetoken(serverResponse);
			checktoken();
			var menuhtml="<H1 style=\"overflow-wrap:break-word; max-width:80%; word-break:break-all;\"><b>Choose one option from the top menu</b></H1>";
			if(menu_login) menu_login.innerHTML = menuhtml;
			if(menu_login) menu_login.style.display = "block";
		}else{
			repo_logout();
// 			checktoken();
			repo_load_menu_login();
			if(menu_login) menu_login.style.display = "block";
			var serverResponse = xhr.responseText;
			if(menu_phantom) menu_phantom.style.display = "none";
			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>Error: "+ serverResponse+ "</pre>";
			if(debug_phantom) debug_phantom.style.display = "block";
		}
	};
	xhr.send(null);
	return false;
}

function exec_login(user,password){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var url = build_execman_path() +"/login?email="+user+"\&pw="+password+"";//?pretty='true'";
	var xhr = new XMLHttpRequest();
	xhr.open("GET", url, true);
	xhr.onreadystatechange = function() {
		if (xhr.status == 200) {
			var serverResponse = xhr.responseText;
			savetoken(serverResponse);
			checktoken();
		}else{
			var serverResponse = xhr.responseText;
			if(demoreplaceb) demoreplaceb.innerHTML = "<pre>Error: "+ serverResponse+ "</pre>";
			if(debug_phantom) debug_phantom.style.display = "block";
			exec_logout();
			checktoken();
		}
	};
	xhr.send(null);
	return false;
}


function jsontotable_repo_logs_brief(myjson,count,first,level,lastwascoma,mtitle,filtered_fields){
	var html ="";
	var i;
// 	if(first==true){ html ="{"; }
	var mainc=mtitle;
	if(first==true){
		html += "<div><table style='border:1px solid black' id=\"table_results\">\n";// style='width:100%'>";
		html += "<th><strong> _id </strong> </th>\n";
		html += "<td><strong> &nbsp;Code&nbsp; </strong></td>\n";
		html += "<td><strong> &nbsp;User&nbsp; </strong></td>\n";
		html += "<td><strong> &nbsp;Ip&nbsp; </strong></td>\n";
		html += "<td><strong> &nbsp;Message&nbsp; </strong></td>\n";
		html += "<td><strong> &nbsp;Date&nbsp;</strong></td>\n";
		count++;
	}
	first=false;
	var countseries=0;
	myjson.forEach(function(val) {
// 		if (count != 1 && lastwascoma==false) {
// 			if(countseries==0) {
// 				html += ",<br>";
// 			}else{
// 				html += "<br>},{<br>";
// 			}
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
							html += "</tr>\n<tr>";
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<td> " + val['_id'] +" </td>\n";
						//source
						if(val['_source'] !=undefined){
							if(val['_source']['code']==undefined){
								html += "<td></td>\n";
							}else if((val['_source']['code']>="200")&&(val['_source']['code']<"300")){//green 2xx-correct,3xx-redirections
								html += "<td bgcolor=\"#00ff00\"> <font color=\"black\">" + val['_source']['code'] +"</font> </td>\n";
							}else if((val['_source']['code']>="400") && (val['_source']['code']<"600")) {//red 4xx-client-error 5xx-server-error
								html += "<td bgcolor=\"#ff3e29\"> <font color=\"black\">" + val['_source']['code'] +"</font> </td>\n";
							}else if((val['_source']['code']>="100")&&(val['_source']['code']<"200")){ //yellow-information
								html += "<td bgcolor=\"#f3ff3a\"> <font color=\"black\">" + val['_source']['code'] +"</font></td>\n";
								
	// 						}else if(val['_source']['code']=="cancelled"){//red
	// 							html += "<td bgcolor=\"#ff3e29\"> " + val['_source']['code'] +" </td>\n";
	// 						}else if(val['_source']['code']=="started"){//green
	// 							html += "<td bgcolor=\"#00FF00/*\*/">" + val['_source']['code'] +"</td>\n";
	// 						}else{
	// // 							html += "<td> " + val['_source']['code'] +"</td>\n";
							}
						
							if(val['_source']['user']==undefined){
									html += "<td></td>\n"; 
							}else{
								html += "<td> " + val['_source']['user'] +" </td>\n";
							}
							if(val['_source']['ip']==undefined){
									html += "<td></td>\n"; 
							}else{
								html += "<td> " + val['_source']['ip'] +" </td>\n";
							}
							if(val['_source']['message']==undefined){
									html += "<td></td>\n"; 
							}else{
								html += "<td> " + val['_source']['message'] +" </td>\n";
							}
							if(val['_source']['date']==undefined){
									html += "<td></td>\n"; 
							}else{
								html += "<td> " + val['_source']['date'] +" </td>\n";
							}
						}else{
							html += "<td></td>\n";
							html += "<td></td>\n";
							html += "<td></td>\n";
							html += "<td></td>\n";
							html += "<td></td>\n";
						}
						mtitle=false;
						count++;
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
// // 					if (count != 1) html += ',<br>';
// // 					for (i = 0; i < level; i++) {
// // 						if (count != 1) html += '&emsp;';
// // 					}
// 					if(mtitle==true){
// 						if(count>1){
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
// 						}
// 						html += "<tr><th><strong>\"" + key + "\"</strong>: </th>\n";
// 						
// 						mtitle=false;
// 					}else{
// 						html += "<tr><td><strong>\"" + key + "\"</strong>: </td>\n";
// 					}
// 					count++;
// 					lastwascoma=false;
// 					html += "<td><div><table style='width:100%; border:0px solid black'>\n";// style='width:100%'>";
					html += jsontotable_repo_logs_brief( ([ val[key] ]), count, first, level+1 ,lastwascoma,mtitle,filtered_fields);
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


//_filter_workflow_taskid_experimentid
function jsontotable_exec_brief(myjson,count,first,level,lastwascoma,mtitle,filtered_fields){
	var html ="";
	var i;
// 	if(first==true){ html ="{"; }
	var mainc=mtitle;
	if(mtitle==true){
		html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
			html += "<th><strong> execution_id </strong> </th>\n";
			html += "<td><strong> Req status </strong></td>\n";
		html += "<td><strong> Project </strong></td>\n";
		html += "<td><strong> Map </strong></td>\n";
		html += "<td><strong> Requested-by</strong></td>\n";
		html += "<td><strong> Input </strong></td>\n";
		html += "<td><strong> Request date </strong></td>\n";
		html += "<td><strong>Start timestamp</strong></td>\n";
		html += "<td><strong>End timestamp</strong></td>\n";
		count++;
	}
	var countseries=0;
	myjson.forEach(function(val) {
// 		if (count != 1 && lastwascoma==false) {
// 			if(countseries==0) {
// 				html += ",<br>";
// 			}else{
// 				html += "<br>},{<br>";
// 			}
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
							html += "</tr>\n<tr>";
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<th> " + val['execution_id'] +" </th>\n";
						if(val['req_status']=="pending"){ //yellow
							html += "<td bgcolor=\"#f3ff3a\"> " + val['req_status'] +" </td>\n";
						}else if(val['req_status']=="completed"){//green
							html += "<td bgcolor=\"#00FF00\"> " + val['req_status'] +" </td>\n";
						}else if(val['req_status']=="cancelled"){//red
							html += "<td bgcolor=\"#ff3e29\"> " + val['req_status'] +" </td>\n";
						}else if(val['req_status']=="started"){//green
							html += "<td bgcolor=\"#00FF00\">" + val['req_status'] +"</td>\n";
						}else{
							html += "<td> " + val['req_status'] +"</td>\n";
						}
						html += "<td>" + val['project'] +" </td>\n";
						html += "<td>" + val['map'] +" </td>\n";
						html += "<td>" + val['requested-by'] +" </td>\n";
						if (val['input']!=undefined){
							html += "<td>" + val['input'] +" </td>\n";
						}else{
							html += "<td></td>\n";
						}
						html += "<td>" + val['req_date'] +" </td>\n";
						if (val['start_timestamp']!=undefined){
							html += "<td>" + val['start_timestamp'] +" </td>\n";
						}else{
							html += "<td></td>\n";
						}
						if (val['end_timestamp']!=undefined){
							html += "<td>" + val['end_timestamp'] +" </td>\n";
						}else{
							html += "<td></td>\n";
						}
						mtitle=false;
						count++;
						lastwascoma=false;
					}
					if((key=="rejection_reason")){
						if(val['req_status']=="rejected"){
							html += "<td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td>\n";
							count++;
							lastwascoma=false;
						}
					}else if((key!="req_status")&&(key!="energy")
						&&(key!="execution_id")&&(key!="app")&&(key!="device")
						&&(key!="project")
						&&(key!="map")
						&&(key!="requested-by")
						&&(key!="input")
						&&(key!="req_date")
						&&(key!="start_timestamp")
						&&(key!="end_timestamp")){
						html += "<td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td>\n";
						count++;
						lastwascoma=false;
					}
				}
			}else if (getType(val[key]) == "array" || getType(val[key]) == "object" ) {
				if(key!= "component_stats"){
// 					if (count != 1) html += ',<br>';
// 					for (i = 0; i < level; i++) {
// 						if (count != 1) html += '&emsp;';
// 					}
					if(mtitle==true){
						if(count>1){
							html += "</table></div></td><br>\n";
							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<tr><th><strong>\"" + key + "\"</strong>: </th>\n";
						mtitle=false;
					}else{
						html += "<tr><td><strong>\"" + key + "\"</strong>: </td>\n";
					}
					count++;
					lastwascoma=false;
					html += "<td><div><table style='width:100%; border:0px solid black'>\n";// style='width:100%'>";
					html += jsontotable( ([ val[key] ]), count, true, level+1 ,lastwascoma,mtitle,filtered_fields);
					html += "</table></div></td>\n";
				}
// 			}else if (getType(val[key]) == "object" ) {
// 				html += jsontotable( ([ val[key] ]), count, false, level+1,lastwascoma,mtitle,filtered_fields);
			};
		});
		mtitle=true;
		countseries++;
	});
// 	if(first==true){ html += "<br>}"; }
	if(mainc==true)
		html += "</table></div>\n";
	return html;
}//jsontotable_exec_brief



//_filter_workflow_taskid_experimentid
function jsontotable_app_brief(myjson,count,first,level,lastwascoma,mtitle,filtered_fields){
	var html ="";
	var i;
// 	if(first==true){ html ="{"; }
	var mainc=mtitle;
	if(mtitle==true){
		html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
		html += "<td><strong><center>id</center></strong></td><th><strong>&nbsp; Project &nbsp;</strong> </th>\n";
		html += "<td><strong>&nbsp; Development &nbsp;</strong></td>\n";
		html += "<td><strong>&nbsp; PT code analysis&nbsp;</strong></td>\n";
		html += "<td><strong>&nbsp; MBT early validation &nbsp;</strong></td>\n";
		html += "<td><strong>&nbsp; IP core generator&nbsp;</strong></td>\n";
		html += "<td><strong>&nbsp; MOM &nbsp;</strong></td>\n";
		count++;
	}
	var countseries=0;
	myjson.forEach(function(val) {
// 		if (count != 1 && lastwascoma==false) {
// 			if(countseries==0) {
// 				html += ",<br>";
// 			}else{
// 				html += "<br>},{<br>";
// 			}
// 		};//this is not the first element
		lastwascoma=true;
		var keys = Object.keys(val);
		keys.forEach(function(key) {
// 			if (getType(val[key]) == "string" || getType(val[key]) == "other" ){
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
							html += "</tr>\n<tr>";
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<td> " + val['_id'] +" </td>\n";
						html += "<th> &nbsp;" + val['project'] +"&nbsp; </th>\n";
						//source
						if(val['source']!=undefined){
							if(val['source']['status']==undefined){
								html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
							}else if(val['source']['status']=="waiting"){ //yellow
								html += "<td bgcolor=\"#f3ff3a\">&nbsp;" + val['source']['status'];
							}else if(val['source']['status']=="finished"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['source']['status'];
							}else if(val['source']['status']=="cancelled"){//red
								html += "<td bgcolor=\"#ff3e29\">&nbsp;" + val['source']['status'];
							}else if(val['source']['status']=="started"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['source']['status'];
							}else{
								html += "<td>&nbsp;" + val['source']['status'];
							}
						}else{
							html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
						}
						html += "&nbsp;</td>\n";
						//pt
						if(val['pt_ca']!=undefined){
							if(val['pt_ca']['status']==undefined){
									html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
							}else if(val['pt_ca']['status']=="waiting"){ //yellow
								html += "<td bgcolor=\"#f3ff3a\">&nbsp;" + val['pt_code_analysis']['status'];
							}else if(val['pt_ca']['status']=="finished"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['pt_code_analysis']['status'];
							}else if(val['pt_ca']['status']=="cancelled"){//red
								html += "<td bgcolor=\"#ff3e29\">&nbsp;" + val['pt_code_analysis']['status'];
							}else if(val['pt_ca']['status']=="started"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['pt_code_analysis']['status'];
							}else{
								html += "<td>&nbsp;" + val['pt']['status'];
							}
						}else{
							html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
						}
						html += "&nbsp;</td>\n";
						//mbt_early_validation
						if(val['mbt_early_validation']!=undefined){
							if(val['mbt_early_validation']['status']==undefined){
									html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
							}else if(val['mbt_early_validation']['status']=="waiting"){ //yellow
								html += "<td bgcolor=\"#f3ff3a\">&nbsp;" + val['mbt_early_validation']['status'];
							}else if(val['mbt_early_validation']['status']=="finished"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['mbt_early_validation']['status'];
							}else if(val['mbt_early_validation']['status']=="cancelled"){//red
								html += "<td bgcolor=\"#ff3e29\">&nbsp;" + val['mbt_early_validation']['status'];
							}else if(val['mbt_early_validation']['status']=="started"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['mbt_early_validation']['status'];
							}else{
								html += "<td>&nbsp;" + val['mbt_early_validation']['status'];
							}
						}else{
							html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
						}
						html += "&nbsp;</td>\n";
						//ip_core_generator
						if(val['ip_core_generator']!=undefined){
							if(val['ip_core_generator']['status']==undefined){
									html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
							}else if(val['ip_core_generator']['status']=="waiting"){ //yellow
								html += "<td bgcolor=\"#f3ff3a\">&nbsp;" + val['ip_core_generator']['status'];
							}else if(val['ip_core_generator']['status']=="finished"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['ip_core_generator']['status'];
							}else if(val['ip_core_generator']['status']=="cancelled"){//red
								html += "<td bgcolor=\"#ff3e29\">&nbsp;" + val['ip_core_generator']['status'];
							}else if(val['ip_core_generator']['status']=="started"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['ip_core_generator']['status'];
							}else{
								html += "<td>&nbsp;" + val['ip_core_generator']['status'];
							}
						}else{
							html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
						}
						html += "&nbsp;</td>\n";
						//mom
						if(val['mom'] !=undefined){
							if(val['mom']['status']==undefined){
									html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
							}else if(val['mom']['status']=="waiting"){ //yellow
								html += "<td bgcolor=\"#f3ff3a\">&nbsp;" + val['mom']['status'];
							}else if(val['mom']['status']=="finished"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['mom']['status'];
							}else if(val['mom']['status']=="cancelled"){//red
								html += "<td bgcolor=\"#ff3e29\">&nbsp;" + val['mom']['status'];
							}else if(val['mom']['status']=="started"){//green
								html += "<td bgcolor=\"#00FF00\">&nbsp;" + val['mom']['status'];
							}else{
								html += "<td>&nbsp;" + val['mom']['status'];
							}
						}else{
							html += "<td bgcolor=\"#f3ff3a\">&nbsp;waiting";
						}
						html += "&nbsp;</td>\n";
						mtitle=false;
						count++;
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
// 			}else if (getType(val[key]) == "array" || getType(val[key]) == "object" ) {
// 				if(key!= "component_stats"){
// // 					if (count != 1) html += ',<br>';
// // 					for (i = 0; i < level; i++) {
// // 						if (count != 1) html += '&emsp;';
// // 					}
// 					if(mtitle==true){
// 						if(count>1){
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
// 						}
// 						html += "<tr><th><strong>\"" + key + "\"</strong>: </th>\n";
// 						
// 						mtitle=false;
// 					}else{
// 						html += "<tr><td><strong>\"" + key + "\"</strong>: </td>\n";
// 					}
// 					count++;
// 					lastwascoma=false;
// 					html += "<td><div><table style='width:100%; border:0px solid black'>\n";// style='width:100%'>";
// 					html += jsontotable( ([ val[key] ]), count, true, level+1 ,lastwascoma,mtitle,filtered_fields);
// 					html += "</table></div></td>\n";
// 				}
// // 			}else if (getType(val[key]) == "object" ) {
// // 				html += jsontotable( ([ val[key] ]), count, false, level+1,lastwascoma,mtitle,filtered_fields);
// 			};
		});
		mtitle=true;
		countseries++;
	});
// 	if(first==true){ html += "<br>}"; }
	if(mainc==true)
		html += "</table></div>\n";
	return html;
}//jsontotable_app_brief


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
// 			if(countseries==0) {
// 				html += ",<br>";
// 			}else{
// 				html += "<br>},{<br>";
// 			}
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
						mtitle=false;
					}else{
						html += "<tr><td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td></tr>\n";
					}
					count++;
					lastwascoma=false;
				}
			}else if (getType(val[key]) == "array" || getType(val[key]) == "object" ) {
// 					if (count != 1) html += ',<br>';
// 					for (i = 0; i < level; i++) {
// 						if (count != 1) html += '&emsp;';
// 					}
					if(mtitle==true){
						if(count>1){
							html += "</table></div></td><br>\n";
							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
						}
						html += "<tr><th><strong>\"" + key + "\"</strong>: </th>\n";
						
						mtitle=false;
					}else{
						html += "<tr><td><strong>\"" + key + "\"</strong>: </td>\n";
					}
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
// 	if(first==true){ html += "<br>}"; }
	if(mainc==true)
		html += "</table></div>\n";
	return html;
}//jsontotable

function jsontotable_only_device_names(myjson,count,first,level,lastwascoma,mtitle,fields_toshow){
	var html ="";
	var i;
	var j=0;
	var previous_val_key="";
// 	if(first==true){ html ="{"; }
	var mainc=false;
	if(mtitle==true && level==1){
		mainc=true;
		html += "<div><table>\n";// style='border:1px solid black'>\n";// style='width:100%'>";
	}
	var countseries=0;
	myjson.forEach(function(val) {
// 		if (count != 1 && lastwascoma==false) {
// 			if(countseries==0) {
// 				html += ",<br>";
// 			}else{
// 				html += "<br>},{<br>";
// 			}
// 		};//this is not the first element
		lastwascoma=true;
		var keys = Object.keys(val);
		keys.forEach(function(key) {
			if (getType(val[key]) == "string" || getType(val[key]) == "other" ){//other can be a numeric value
				var tobefiltered=true;
				for (i=0;i< fields_toshow.length;i++){
					if (key.endsWith(fields_toshow[i], key.length)== true) {
						tobefiltered=false;
					}
				}
				if (tobefiltered== false) {//it is stored the length of the strings, not need to show
// 					if (count != 1 && lastwascoma==false) html += ',<br>';
// 					for (i = 0; i < level; i++) {
// 						if (count != 1) html += '&emsp;';
// 					}
					if(mtitle==true){
// 						if(count>1){
// 							html += "</table></div></td><br>\n";
// 							html += "<div><table style='border:1px solid black'>\n";// style='width:100%'>";
// 						}
						if ((count==1) || ((count>1) && (previous_val_key!=val[key]))){
						html += "<tr><th><strong>\""+ key +"\"</strong>: \"" + val[key] +"\"</th></tr>\n";
						}
						previous_val_key=val[key];
						mtitle=false;
// 					}else{
// 						html += "<tr><td><strong>\"" + key +"\"</strong>: \"" + val[key] +"\"</td></tr>\n";
					}
					count++;
					lastwascoma=false;
				}
			}else if (getType(val[key]) == "array" ) {
// 					if (count != 1) html += ',<br>';
// 					for (i = 0; i < level; i++) {
// 						if (count != 1) html += '&emsp;';
// 					}
// 					if(mtitle==true){
// 						html += "<tr><th><strong>\"" + key + "\"</strong>: </th>\n";
// 						mtitle=false;
// 					}else{
// 						html += "<tr><td><strong>\"" + key + "\"</strong>: </td>\n";
// 					}
// 					count++;
					lastwascoma=false;
// 					html += "<td><div><table style='width:100%; border:0px solid black'>\n";// style='width:100%'>";
					html += jsontotable_only_device_names( ([ val[key] ]), count, true, level+1 ,lastwascoma,mtitle,fields_toshow);
// 					html += "</table></div></td>\n";
			}else if (getType(val[key]) == "object" ) {
				html += jsontotable_only_device_names( ([ val[key] ]), count, false, level+1,lastwascoma,mtitle,fields_toshow);
			};
		});
		mtitle=true;
		countseries++;
	});
// 	if(first==true){ html += "<br>}"; }
	if(mainc==true)
		html += "</table></div>\n";
	return html;
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
	}else {
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
	}
	return false;
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

function download_file(content, fileName, contentType) {
// 	var file = new Blob([str2bytes(content)], {type: contentType});
	var file = new Blob([content], {type: contentType});
// 	if (navigator.msSaveOrOpenBlob) {
// 		navigator.msSaveOrOpenBlob(file, filename);
// 	} else {
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


function request_downloadzip(url, outputfile, type){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var phantom_operation = document.getElementById("phantom_operation");
	var xhr = new XMLHttpRequest();
	console.log("url is "+url);
	xhr.open("GET", url, true);
	xhr.processData= false;
	xhr.setRequestHeader("Content-Type", type);
	if(sessionStorage.token !== undefined){
		if(sessionStorage.token.length>0) {
			xhr.setRequestHeader("Authorization", "JWT " + sessionStorage.token);
	}}
	xhr.responseType = 'arraybuffer';
	xhr.addEventListener('load', function() {
		if (xhr.readyState === 4 && xhr.status == 200) {
// 			var responseObject = [ xhr.response ];
			if(outputfile.length>0)
				download_file(xhr.response, outputfile, type);
			if (demoreplaceb) demoreplaceb.innerHTML = "Downloaded file."; //+responseObject + " status: " +xhr.status;
			if (debug_phantom) debug_phantom.style.display = "block";
			if(phantom_operation) phantom_operation.style.display="none";
		}
	});
	xhr.send(null);
	return false;
}

function request_download(url, outputfile, type){
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	var phantom_operation = document.getElementById("phantom_operation");
	var xhr = new XMLHttpRequest();
	console.log("url is "+url);
	xhr.open("GET", url, true);
	xhr.processData= false;	
	xhr.setRequestHeader("Content-Type", type);
	if(sessionStorage.token !== undefined){
		if(sessionStorage.token.length>0) {
			xhr.setRequestHeader("Authorization", "JWT " + sessionStorage.token);
	}}
	xhr.addEventListener('load', function() {
// 	xhr.onreadystatechange = function() {
		if (xhr.readyState === 4 && xhr.status == 200) {
			var responseObject = [ xhr.responseText ];
			if(outputfile.length>0)
				download_file(xhr.response, outputfile, type);
			if (demoreplaceb) demoreplaceb.innerHTML = "<pre>" + responseObject+ "</pre>"; // + " status: " +xhr.status;
			if (debug_phantom) debug_phantom.style.display = "block";
			if(phantom_operation) phantom_operation.style.display="none";
		}else{
			if (demoreplaceb) demoreplaceb.innerHTML = "<pre>"+responseObject + " status: " +xhr.status + "</pre>";
			if (debug_phantom) debug_phantom.style.display = "block";
		}
	});
	xhr.send(null);
	return false;
}

function list_results(mytype,url,fields_toshow,filtered_fields){
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
				console.log("myjsob "+JSON.stringify(myjson));
				myjson = myjson.hits;
			}else{
				myjson = [ myjson ];
			}
			if(myjson!=undefined) {
				if (mytype== 1) {
					html += jsontotable(myjson,1,true,1,false,true,filtered_fields);
				}else if (mytype == 20){//repository
					html += jsontotable_repo_logs_brief(myjson,1,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if (mytype == 21){//app manager
					html += jsontotable_repo_logs_brief(myjson,1,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if (mytype == 22){//exec,manager
					html += jsontotable_repo_logs_brief(myjson,1,true,1,false,true,filtered_fields);
					html += "</table></div>\n";
				}else if (mytype == 23){//monitoring server
					html += jsontotable_repo_logs_brief(myjson,1,true,1,false,true,filtered_fields);
					html += "</table></div>\n";					

				}else if (mytype == 24){//resource manager
					html += jsontotable_repo_logs_brief(myjson,1,true,1,false,true,filtered_fields);
					html += "</table></div>\n";								
					
					
					
				}else if (mytype == 5){
					html += jsontotable_exec_brief(myjson,1,true,1,false,true,filtered_fields);
				}else if (mytype == 6){
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

function upload_app_with_token( UploadJSON ) {
	var url = build_appman_path() + "/register_new_project";
	upload_with_token( UploadJSON, url);
	return false;
}

function upload_device_with_token( UploadJSON ) {
	var url=build_resource_path()+"/register_new_device";
	upload_with_token( UploadJSON ,url);
	return false;
}

function upload_exec_with_token( UploadJSON ) {
	var url = build_execman_path() +"/register_new_exec";
	upload_with_token( UploadJSON ,url);
	return false;
}

function update_app_with_token( UploadJSON ) {
	var url = build_appman_path() + "/update_project_tasks";
	upload_with_token( UploadJSON ,url);
	return false;
}

function update_device_with_token( UploadJSON ) {
	var url=build_resource_path()+"/update_device";
	upload_with_token( UploadJSON ,url);
	return false;
}

function update_mf_config_with_token( UploadJSON ) {
	var url=build_resource_path()+"/register_mf_config";
// 	var url=build_resource_path()+"/update_device_status";
	upload_with_token( UploadJSON ,url);
	return false;
}

function upload_mf_config_with_token( UploadJSON ) {
	var url=build_resource_path()+"/register_mf_config";
// 	var url=build_resource_path()+"/update_device_status";
	upload_with_token( UploadJSON ,url);
	return false;
}

function update_exec_with_token( UploadJSON ) {
	var url = build_execman_path() +"/update_exec";
	upload_with_token( UploadJSON ,url);
	return false;
}

function list_results_with_token( mytype ,url,fields_toshow, filtered_fields) {
	var demoreplaceb = document.getElementById("demoreplaceb");
	var debug_phantom = document.getElementById("debug_phantom");
	if((sessionStorage.token) && (sessionStorage.token.length>0)) {//reject null, undefined and empty string
		list_results(mytype,url,fields_toshow,filtered_fields);
	}else {
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
	}
	return false;
}


function list_apps(mytype,appname){
	var url = build_appman_path() + "/get_app_list?project=\""+appname+"\"";//?pretty='true'";
	list_results(mytype,url,["_id"],["_length"]);
	return false;
}

function list_repo_logs(mytype,user){
	var url = build_repo_path() + "/get_log_list?pretty='true'";
	list_results(mytype,url,["host"],["_length","_index","_type","_score","sort"]);
	return false;
}


function list_app_logs(mytype,appname){// will list projects
	var url = build_appman_path() + "/get_log_list?pretty='true'";
	list_results(mytype,url,["host"],["_length","_index","_type","_score","sort"]);
	return false;
}


function list_exec_logs(mytype,execid){
	var url = build_execman_path() + "/get_log_list?pretty='true'";
	list_results(mytype,url,["host"],["_length","_index","_type","_score","sort"]);
	return false;
}

function list_execs(mytype,execname){
	var url = build_execman_path() +"/get_exec_list?app=\""+execname+"\"";//?pretty='true'";
	list_results(mytype,url,["host"],["_length"]);
	return false;
}

function list_devices(mytype,devicename){
	var url=build_resource_path()+"/get_device_list?device=\""+devicename+"\"";//?pretty='true'";
	list_results(mytype,url,["device"],["_length"]);
	return false;
}

function list_exps(mytype,devicename){
	var url=build_monitoring_path()+"/get_device_list?device=\""+devicename+"\"";//?pretty='true'";
	list_results(mytype,url,["device"],["_length"]);
	return false;
}

function list_status_devices(mytype,devicename){
	// get_plugin_status localhost 9400 "node01";
	var url=build_resource_path()+"/query_device_status?device=\""+devicename+"\"";//?pretty='true'"; 
	list_results_with_token(mytype,url,["host"],["_length","WorkflowID", "ExperimentID", "TaskID"] );
	return false;
}

function list_mf_config_devices(mytype,devicename){
	// get_plugin_status localhost 9400 "node01";
	var url=build_resource_path()+"/query_device_mf_config?device=\""+devicename+"\"";//?pretty='true'";
	list_results_with_token(mytype,url,["platform_id"],["_length"] );
	return false;
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
				} else {
					r = "Error " + xhr.status + " occurred requesting for Metatada.<br/>";
				}
				if(demoreplaceb) document.getElementById("demoreplaceb").innerHTML = "<pre>"+r+"</pre>";
				if(debug_phantom) document.getElementById("debug_phantom").style.display = "block";
				if(phantom_operation) document.getElementById("phantom_operation").style.display = "none";
			}
		});
		xhr.send(null);//may fault code appear here
	}else {
		if(demoreplaceb) demoreplaceb.innerHTML = "Sorry, try login again, missing token...";
		if(debug_phantom) debug_phantom.style.display = "block";
	}
	return false;
}

function submitform_qr_metatada(e, frm) {
	var filepath = document.getElementById("Path").value;
	var filename = document.getElementById("filename").value;
	var project= document.getElementById("project").value;
	var source = document.getElementById("source").value;
	var url = "query_metadata?project=" + project + "&source=" + source + "&filepath=" + filepath + "&filename=" + filename;
	submitform(url, 'GET', 'metadata.json');
	return false;
}

function submitform_qr_metatada_es(e, frm) {
	var QueryBody = document.getElementById("QueryBody").value;
	var url = "es_query_metadata?QueryBody=" + QueryBody;
	submitform(url, 'GET', 'metadata.json');
	return false;
}

function submitform_file_list(project, source,filepath){
	if(project == undefined){
		return false;
	}else if(project.length==0){
		return false;
	}
	var url = build_repo_path() + "/downloadlist?project=\""+project+"\"";
	if(source !== undefined){
		if(source.length>0) {
			url +="\&source=\""+source+"\"";
			if(filepath !== undefined){
				if(filepath.length>0) {
					url +="\&filepath=\""+filepath+"\"";
				}
			}
		}
	}
	submitform(url, 'GET', '');
	return false;
}

function download_file_repo(project, source,filepath, filename){
	var url = build_repo_path() + "/download?project=\""+project+"\"\&source=\""+source+"\"\&filepath=\""+filepath+"\"\&filename=\""+filename+"\"";//?pretty='true'";
	request_download(url, "", 'text/plain');
	return false;
}

function download_metadata_repo(project,source,filepath, filename){
	var url = build_repo_path() + "/query_metadata?project=\""+project+"\"\&source=\""+source+"\"\&filepath=\""+filepath+"\"\&filename=\""+filename+"\"";//?pretty='true'";
	request_download(url, "", 'text/plain');
	return false;
}

function downloadzip_file_repo(project, source, filepath, filename){
	if(project == undefined){
		return false;
	}else if(project.length==0){
		return false;
	}
	var url = build_repo_path() + "/downloadzip?project=\""+project+"\"";
	if(source !== undefined){
		if(source.length>0) {
			url +="\&source=\""+source+"\"";
			if(filepath !== undefined){
				if(filepath.length>0) {
					url +="\&filepath=\""+filepath+"\"";
					if(filename !== undefined){
						if(filename.length>0) {
							url +="\&filename=\""+filename+"\"";
						}
					}
				}
			}
		}
	}
	request_downloadzip(url, "output.zip", "application/zip");
	return false;
}
