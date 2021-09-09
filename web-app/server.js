'use strict'

const express = require('express')
const app = express()

const port = 8888

const firebaseCreator = require('./firebase.js')
const firebase = firebaseCreator()

const serviceCreator = require('./services.js')
const service = serviceCreator(firebase)

const webapiCreator = require('./web-api.js')
const webapi = webapiCreator(service)

const webuiCreator = require('./web-ui.js')
const webui = webuiCreator(service)

app.use('/api', webapi)
app.use(webui);

app.set('view engine', 'hbs')

app.listen(port, () => console.log(`listening on port: ${port}`))