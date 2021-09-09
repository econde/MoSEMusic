'use strict'

const express = require('express')


function webui(service) {

    const theWebUI = {

        today: (req, res) => {
            service.getInfo()
                .then(data => {
                    const answer = {
                        'info': data
                    }
                    res.render('today', answer)
                })
                .catch(data => {
                    const answer = {
                        'info': data
                    }
                    res.render('error', answer)
                })

        },
        home: (req, res) => {
            res.redirect('/Today')
        },
        search: (req, res) => {
            res.render('search')
        },
        getStatistics: (req, res) => {
            res.render('statistics')
        },
        searchEvent: (req, res) => {
            service.getInfoFromDayandEvent(req.query.eventname, req.query.date)
                .then(data => {
                    const answer = {
                        'info': data
                    }
                    res.render('dateevent', answer)
                })
                .catch(data => {
                    const answer = {
                        'info': data
                    }
                    res.render('error', answer)
                })
        },
        eventStats: (req, res) => {
            service.eventStats(req.query.eventname, req.query.date)
                .then(data => {
                    const answer = {
                        'info': data
                    }
                    res.render('showStats', answer)
                })
                .catch(data => {
                    const answer = {
                        'info': data
                    }
                    res.render('error', answer)
                })
        },
        about: (req, res) => {
            res.render('about')
        },
        help: (req, res) => {
            res.render('help')
        }
    }

    const router = express.Router();
    router.use(express.urlencoded({
        extended: true
    }))

    router.get('/Today', theWebUI.today)
    router.get('/', theWebUI.home)
    router.get('/Search', theWebUI.search)
    router.get('/Stats', theWebUI.getStatistics)
    router.get('/SearchEvent', theWebUI.searchEvent)
    router.get('/EventStats', theWebUI.eventStats)
    router.get('/Help', theWebUI.help)
    router.get('/About', theWebUI.about)
    return router;
}

module.exports = webui