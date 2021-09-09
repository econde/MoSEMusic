'use strict'


function service(firebase) {

    const theService = {

        getInfo: () => {

            return firebase.getInfo().then(info => {
                if (info === null) {
                    const str = "Informação indisponível"
                    return Promise.reject(str)
                }
                return info
            })
        },
        getInfoFromDayandEvent: (eventname, date) => {
            return firebase.getInfoFromDayandEvent(eventname, date).then(info => {
                if (info === null) {
                    const str = "Informação indisponível"
                    return Promise.reject(str)
                }
                return info
            })
        },
        eventStats: (eventname, date) => {
            return firebase.eventStats(eventname, date).then(info => {
                if (info === null) {
                    const str = "Informação indisponível"
                    return Promise.reject(str)
                }
                return info
            })
        }

    }
    return theService

}

module.exports = service