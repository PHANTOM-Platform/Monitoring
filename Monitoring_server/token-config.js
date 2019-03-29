//Codificando el JSON Web Token
//Ahora vamos al "meollo" de este artículo, la creación de un Token que identifique a nuestro usuario en cada petición HTTP que realice.

//Para codificar el token utilizamos una clave secreta. Es importante que esta clave permanezca lo más oculta posible. Una opción es almacenarla en un fichero config.js y ese fichero no subirlo al repositorio con .gitignore o la opción mejor es utilizar una variable de entorno (con process.env) que esté en nuestro servidor, y otra para nuestro entorno de desarrollo.


// config.js
module.exports = {
  TOKEN_SECRET: process.env.TOKEN_SECRET || "tokenultrasecreto"
};

//Cuando importemos el fichero config.js en la variable TOKEN_SECRET tendremos nuestra clave para codificar.


