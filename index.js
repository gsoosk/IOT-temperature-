//----------------------------------------------------------------------------
//------------------------------  Server Code  -------------------------------
//----------------------------------------------------------------------------

//************************** Including libraries *****************************

const fs = require('fs'); //file system
const express = require('express'); //web service
const bodyParser = require('body-parser'); //client to server

//*************************** Functions Declaration **************************

function showPage(authen, res){
  //console.log(res);
   if(authen)
      res.sendFile(__dirname + '/pages/home.html')
   else
      res.sendFile(__dirname + '/pages/login.html')
}

//************************** Variables Definition ****************************

var auth = false;
var port = 3100; //env_port.process;

//******************************  Use express  *******************************

app = express()
app.use(express.static(__dirname + '/public'))
app.use(bodyParser.urlencoded({ extended: false }))
app.use(bodyParser.json())

//----------------------------------------------------------------------------
//------------------------------  Main Code ----------------------------------
//----------------------------------------------------------------------------

//**************************** Server is Listening ***************************

app.listen(port, () => {
   console.log(`server is online on port ${port}`);
})

//************************** Response to login request ***********************

app.post('/', (req, res) => {
   // console.log(req.body);
   if((req.body.user == 'farzad') && (req.body.pass == 'spring'))
   {
     console.log(`login`);
     auth = true
   }
   showPage(auth, res)
})

//***************** Response to HomePage Opening request *********************

app.get('/', function(req, res){
   showPage(auth, res)
})

//************************** Response to logout request **********************

app.get('/logout', (req, res) => {
   auth = false
   console.log(`logout`);
   showPage(auth, res)
})

//************************** Response to Log request *************************

app.post('/save', (req, res) => {
   console.log(req.body);
   fs.appendFileSync(__dirname + '/log/log.txt', `${req.body.author || 'no name'} @ ${Date().toString()}:\t${req.body.log}\n`)
    res.redirect('/');
})


//----------------------------------------------------------------------------
//---------------------------------  The End  --------------------------------
//----------------------------------------------------------------------------
