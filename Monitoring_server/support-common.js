//************************************************
	var dateFormat = require('dateformat');
module.exports = {
	//****************************************************
	//This function flush the pending operations to the DataBase.
	my_flush: function( remoteAddress, es_server, my_index){
		var testhttp = require('http');
		var rescode="";
		var myres = { code: "", text: "" };
		var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
		return new Promise( (resolve,reject) => {
			testhttp.get('http://'+es_server+'/'+my_index+'/_flush', function(rescode) {
				myres.code="200";
				myres.text="200 Succeed";
				resolve (myres);
			}).on('error', function(e) {
				myres.code="400";
				myres.text="400"+"Flush error "+currentdate;
				LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, remoteAddress,"Flush error "+e,currentdate,"");
				reject (myres);
			});
		});
	},
	// curl -XPOST localhost:9400/_flush/synced
	synced_flush: function( remoteAddress, es_server, my_index){
		var testhttp = require('http');
		var rescode="";
		var myres = { code: "", text: "" };
		var currentdate = dateFormat(new Date(), "yyyy-mm-dd'T'HH:MM:ss.l");
		return new Promise( (resolve,reject) => {
			testhttp.get('http://'+es_server+'/'+my_index+'/_flush/synced', function(rescode) {
				myres.code="200";
				myres.text="200 Succeed";
				resolve (myres);
			}).on('error', function(e) {
				myres.code="400";
				myres.text="400"+"Flush error "+currentdate;
				LogsModule.register_log(es_servername+":"+es_port,SERVERDB, 400, remoteAddress,"Flush error "+e,currentdate,"");
				reject (myres);
			});
		});
	},
	//**********************************************************
	//This function removes double quotation marks if present at the beginning and the end of the input string
	 remove_quotation_marks: function(input_string){
		if(input_string ==undefined){
			return("");
		}
		if(input_string.charAt(0) === '"') {
			input_string = input_string.substr(1);
		}
		if(input_string.length>0){
		if(input_string.charAt(input_string.length-1) === '"') {
			input_string = input_string.substring(0, input_string.length - 1);
		}}
		return (input_string);
	}
}//end module.exports
