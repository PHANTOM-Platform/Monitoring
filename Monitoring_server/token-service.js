var jwt = require('jwt-simple');
var moment = require('moment');
var config = require('./token-config');
const TokensModule = require('./support-tokens'); 

exports.createToken = function(user_id) {
	var currenttime		= moment().unix();
	var expirationtime	= moment().add(14, "days").unix();
	var result			= TokensModule.register_token(user_id, currenttime, expirationtime);
	result.then((resultreg) => { //it only returns error if not succeed
		var payload = {
			sub: user_id,
			iat: currenttime,
			exp: expirationtime,
		};
		//var token = jwt.encode(payload, config.TOKEN_SECRET); <<-- can not do this !!!
		return jwt.encode(payload, config.TOKEN_SECRET);
	},(resultReject)=> {
		//console.log("Could not register an existing token, please try again."); 
		//console.log("Error registrando token en DB");
		//console.log("log: Bad Request: " + resultReject); 
		return("\n400: Bad Request "+resultReject+"\n");
	} );
};

exports.createToken = function(user_id) {
	var payload = {
		sub: user_id,
		iat: moment().unix(),
		exp: moment().add(30, "days").unix(),
	};
	return jwt.encode(payload, config.TOKEN_SECRET);
};
