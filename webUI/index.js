const password = 'D8';
const PORT = 3001

//Middleware to set up the web server
const {response} = require('express')
const express = require('express')
const app = express()

//Simple "robot" that can programmatically hit keys, use mouse, etc...
var robot = require('robotjs')

//Get the ipAddress for the network that the computer is currently on
var ip = require('ip')
var ipAddress = ip.address()

app.use(express.json())

app.listen(PORT, ipAddress, () => {
    console.log('App started')
})

console.log(`Enter this into the browser on another device: ${ipAddress}:${PORT}`)

//For now the app will just serve a static page for the frontend
//If we go down this route, we will switch to a dedicated frontend framework and create a frontend app
app.use(express.static('public'))

app.post('/api/ctrl', (req, resp) => {
    if(req.body.password === password) {
        console.log('CTRL')
        robot.keyTap('control')

        const status = 'success';
        resp.json({ status });
    }
})

app.post('/api/alt', (req, resp) => {
    if(req.body.password === password) {
        console.log('ALT')
        robot.keyTap('alt')

        const status = 'success';
        resp.json({ status });
    }
})

app.post('/api/esc', (req, resp) => {
    if(req.body.password === password) {
        console.log('ESCAPE')
        robot.keyTap('escape')

        const status = 'success';
        resp.json({ status });
    }
})