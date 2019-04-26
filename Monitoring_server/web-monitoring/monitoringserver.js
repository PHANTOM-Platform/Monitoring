var imported = document.createElement("script");
imported.src = "phantom.js";
document.getElementsByTagName("head")[0].appendChild(imported);

/**
* Returns the host and port (if defined) for building the url
* @returns {String} beginning of the url
*/
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



function mf_logout() {
	sessionStorage.setItem('token', '');
	request_share_session_storage();
// 	checktoken(); //already called at the end of request_share_session_storage
	window.location = 'monitoringserver.html';
	return false;
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




function mf_load_header_footer(){
	mf_load_header();
// 	mf_load_menu_login(); //not implemented login in the mf server
	mf_login("dummy","dummy");
	load_footer();
// 	checktoken();
}


function update_mf_config_with_token( UploadJSON ) {
	var url=build_monitoring_path()+"/register_mf_config";
// 	var url=build_monitoring_path()+"/update_device_status";
	upload_with_token( UploadJSON ,url);
	return false;
}

function upload_mf_config_with_token( UploadJSON ) {
	var url=build_monitoring_path()+"/register_mf_config";
// 	var url=build_monitoring_path()+"/update_device_status";
	upload_with_token( UploadJSON ,url);
	return false;
}

function list_mf_logs(mytype,execid){
	var url = build_monitoring_path() + "/get_log_list?sorttype="+mytype+"&pretty='true'";
	list_results(mytype,url,["host"],["_length","_index","_type","_score","sort"]);
	return false;
}
