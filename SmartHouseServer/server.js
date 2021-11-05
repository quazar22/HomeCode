const express = require('express');
const TuyAPI = require('tuyapi');
var devices = require('./devices_map.json');
var app = express();

app.use(express.json());
app.use(express.urlencoded({ extended: true }));

var port = 8081;
var device_data = [];
var device_objects = {};

for(var i = 0; i < devices.result.length; i++) {
    var device = devices.result[i];
    device_data.push(device);
    deviceSetup(device);
}

function newDeviceObject(device_data)
{
    return new TuyAPI({
        id: device_data.id,
        key: device_data.local_key,
        version: '3.3',
        issueRefreshOnConnect: true
    });
}

function deviceSetup(device_data) 
{
    var device_tuya_object = deviceConnect(device_data);

    device_tuya_object.on('connected', () => {
        console.log('Connected to ' + device_data.name);
    });

    device_tuya_object.on('disconnected', () => {
        console.log('Disconnected from ' + device_data.name);
        deleteDevice(device_data.name);
    });

    device_tuya_object.on('error', error => {
        console.log('Error from: ' + device_data.name + '\n', error);
        deleteDevice(device_data.name); 
    });

    device_tuya_object.on('data', data => {
        console.log('DATA from device: ', device_data.name, ' ', data);
    });
}

function deviceConnect(device) 
{
    var device_tuya_object = device_objects[device.name];
    if(!device_objects[device.name]) {
        device_objects[device.name] = device_tuya_object = newDeviceObject(device);
        device_tuya_object.find().then(() => {
            device_tuya_object.connect();
        }).catch((e) => {
            console.log('error while connecting to new device: ' + device.name);
            deleteDevice(device.name);
        });
    } else {
        device_tuya_object.connect().catch((e) => {
            console.log('error while connecting to current device: ' + device.name);
            deleteDevice(device.name);
        });
    }
    return device_tuya_object;
}

function deleteDevice(deviceName)
{
    //console.log('deleted ' + deviceName);
    delete device_objects[deviceName]; 
}

app.get('/', function(req, res) {
    res.send('there are no get functions :)');
    console.log('somebody is asking for something...');
});

app.post('/', function(req, res) {
    console.log(req.body);

    var command = req.body.command;
    var location = command.location;
    var code = command.code;
    var value = command.value;

    if(device_objects[location] && device_objects[location].isConnected()) 
    {
        device_objects[location].set({dps: code, set: value});
        res.json({success: true});
    } else {
        res.json({success: false});
    }
    
});

app.listen(port, () => {
    console.log(`Example app listening at http://localhost:${port}`)
});

var check_interval = setInterval(() => {
    for(var i = 0; i < device_data.length; i++) {
        var device = device_data[i];
        if(!device_objects[device.name]) {
            deviceSetup(device);
        }
    }
}, 5000);