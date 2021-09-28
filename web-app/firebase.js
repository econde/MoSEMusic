'use strict'
var firebase = require("firebase/app");

// Add the Firebase products that you want to use
require("firebase/auth");
require("firebase/firestore");
require('firebase/database');
var firebaseConfig = {
    //consultar readme.txt
};
firebase.initializeApp(firebaseConfig)
const pref = 2 * Math.pow(10, -5)

function firestore_db() {

    const fb = {

        getInfo: function () {
            let today_date = new Date().toISOString().slice(0, 10)

            return new Promise((resolve, reject) => {
                firebase.database().ref('/' + today_date).on('value', (snapshot) => {
                    const data = snapshot.val()

                    if (data === null) {
                        resolve(null)
                    } else {
                        const evento = Object.keys(data)[0];
                        let json = {
                            "STATION_1": data[evento].STATION_1,
                            "STATION_2": data[evento].STATION_2,
                            "EVENTO": evento
                        }
                        resolve(json)
                    }
                })
            });
        },
        getInfoFromDayandEvent: function (eventname, date) {
            return new Promise((resolve, reject) => {
                firebase.database().ref('/' + date + '/' + eventname).on('value', (snapshot) => {
                    const data = snapshot.val()
                    if (data === null) {
                        resolve(null)
                    } else {
                        let json = {
                            "STATION_1": data.STATION_1,
                            "STATION_2": data.STATION_2,
                            "EVENTO": eventname,
                            "DATA": date
                        }
                        resolve(json)
                    }
                })
            });
        },
        eventStats: function (eventname, date) {
            return new Promise((resolve, reject) => {
                firebase.database().ref('/' + date + '/' + eventname).on('value', (snapshot) => {
                    const data = snapshot.val()
                    if (data === null) {
                        resolve(null)
                    } else {
                        let STATION_1 = null
                        let STATION_2 = null
                        if (data.STATION_1 != null) {
                            let STATION_1_MAX = data.STATION_1.noise_levels.LAmax.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return Math.max(a, b);
                            });
                            STATION_1_MAX = 20 * Math.log10(STATION_1_MAX / pref)

                            let STATION_1_PEAK = data.STATION_1.noise_levels.LApeak.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return Math.max(a, b);
                            });
                            STATION_1_PEAK = 20 * Math.log10(STATION_1_PEAK / pref)

                            let STATION_1_MIN = data.STATION_1.noise_levels.LAmin.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return Math.min(a, b);
                            });
                            STATION_1_MIN = 20 * Math.log10(STATION_1_MIN / pref)

                            let STATION_1_LAE = data.STATION_1.noise_levels.LAE.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return (a + b);
                            }) / data.STATION_1.noise_levels.LAE.length;
                            STATION_1_LAE = 20 * Math.log10(STATION_1_LAE / pref)
                          STATION_1 = {
                                "STATION_1_MAX": Math.round(STATION_1_MAX*10)/10,
                                "STATION_1_MIN": Math.round(STATION_1_MIN*10)/10,
                                "STATION_1_PEAK": Math.round(STATION_1_PEAK*10)/10,
                                "STATION_1_LAE": Math.round(STATION_1_LAE * 10)/10
                            }
                        }
                        if (data.STATION_2 != null) {

                            let STATION_2_MAX = data.STATION_2.noise_levels.LAmax.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return Math.max(a, b);
                            });
                            STATION_2_MAX = 20 * Math.log10(STATION_2_MAX / pref)

                            let STATION_2_PEAK = data.STATION_2.noise_levels.LApeak.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return Math.max(a, b);
                            });
                            STATION_2_PEAK = 20 * Math.log10(STATION_2_PEAK / pref)

                            let STATION_2_MIN = data.STATION_2.noise_levels.LAmin.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return Math.min(a, b);
                            });
                            STATION_2_MIN = 20 * Math.log10(STATION_2_MIN / pref)

                            let STATION_2_LAE = data.STATION_2.noise_levels.LAE.map(log => pref * Math.pow(10, log / 20)).reduce(function (a, b) {
                                return (a + b);
                            }) / data.STATION_2.noise_levels.LAE.length;
                            STATION_2_LAE = 20 * Math.log10(STATION_2_LAE / pref)
                           STATION_2 = {
                                "STATION_2_MAX": Math.round(STATION_2_MAX*10)/10,
                                "STATION_2_MIN": Math.round(STATION_2_MIN*10)/10,
                                "STATION_2_PEAK": Math.round(STATION_2_PEAK*10)/10,
                                "STATION_2_LAE": Math.round(STATION_2_LAE * 10)/10
                            }
                        }
                        const json = {
                            ...STATION_1,
                            ...STATION_2,
                            "EVENTO": eventname,
                            "DATA": date
                        }
                        resolve(json)
                    }
                })
            });
        }
    }
    return fb

}

module.exports = firestore_db