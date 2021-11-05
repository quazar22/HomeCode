const express = require('express');
const TuyAPI = require('tuyapi');
var devices = require('./devices_map.json');
var app = express();

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

var port = 8081;
var device_data = devices.result;
var device_objects = {};

function newDeviceObject(device_info)
{
    return new TuyAPI({
        id: device_info.id,
        key: device_info.local_key,
        version: '3.3',
        issueRefreshOnConnect: true
    });
}

function deviceSetup(device_info, device_tuya_object) 
{
    var found = device_tuya_object.find({all:false});
    device_tuya_object.on('connected', () => {
        console.log('Connected to ' + device_info.name);
    });

    device_tuya_object.on('disconnected', () => {
        console.log('Disconnected from ' + device_info.name);
    });

    device_tuya_object.on('error', error => {
        console.log('Error from: ' + device_info.name + '\n', error);
    });

    device_tuya_object.on('data', data => {
        console.log('DATA from device: ', device_info.name, ' ', data);
    });
}

function deviceConnect(name)
{
    device_objects[name].find({all:false});
    device_objects[name].connect().catch((e) => {
        console.log('error while connecting to current device: ' + name);
    });
}

// var check_interval = setInterval(() => {
//     var names = devices.result.map(device => device.name); //get names of all devices
//     for(var i = 0; i < names.length; i++) 
//     {
//         var name = names[i];
//         if(!device_objects[name] && name === 'office'){
//             var device_info = device_data.filter((device) => (device.name === name))[0];
//             device_objects[name] = newDeviceObject(device_info);
//             deviceSetup(device_info, device_objects[name]);
//         } else {
//             if(device_objects[name] && !device_objects[name]._connected)
//             {
//                 deviceConnect(name);
//             }
//         }
//     }
//     //var t = (new TuyAPI()).find({all:false});
// }, 1000);

office_light = devices.result.filter(device => device.name === 'office')[0];

var t1 = newDeviceObject(office_light);
var t2 = newDeviceObject(office_light);

console.log(t1);
console.log(t2);
function sleep(milliseconds) {
    const date = Date.now();
    let currentDate = null;
    do {
      currentDate = Date.now();
    } while (currentDate - date < milliseconds);
  }

t1.on('connected', () => {
    console.log('Connected to ');
});

t1.on('disconnected', () => {
    console.log('Disconnected from ');
});

t1.on('error', error => {
    console.log('Error from: ');
});

t1.on('data', data => {
    console.log('DATA from device: ');
});

t1.find().then(() => t1.connect()).then(() => t1.disconnect());
sleep(2000);
t1.find().then(() => t1.connect());
console.log('hit here');

