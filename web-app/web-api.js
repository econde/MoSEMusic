'use strict'

const express = require('express')

function webapi(service) {

    const theWebApi = {
        getInfo: (req, res) => {
            service.getInfo()
                .then(info => res.json(info))
            //Missing Catch in case of error
        },
    }
    const router = express.Router();
    router.use(express.json())
    router.get('/info', theWebApi.getInfo)

    return router
}

module.exports = webapi