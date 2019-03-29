var service = require('./token-service');

exports.emailLogin = function(email ) {
	//if valid email and passwd return token
	//register the token in the DataBase for a possible invalidation
	var token= service.createToken(email);
	//console.log("token registrado en DBs "+token+"\n\n");
	return token;
};
