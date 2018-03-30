'use strict';

process.on('unhandledRejection', console.dir);

const mqtt = require('mqtt');

const MQTT_HOST = 'mqtt://localhost:1883';
const client = mqtt.connect(MQTT_HOST);

const minimist = require('minimist');

var args = minimist(process.argv.slice(2), {  
  default: {
    p: 0,
    i: 0,
    _: []
  },
});

const readline = require('readline');

const rl = readline.createInterface({
  input: process.stdin,
  output: process.stdout
});

const SerialPort = require('serialport');
const Readline = SerialPort.parsers.Readline;
const PORT = '/dev/ttyUSB' + args.p;
const sid = args.i
const data = args._ 

const port = new SerialPort(PORT, {
  baudRate: 9600
  },
  err => {
    if (err) {
      return console.log('Error:', err.message);
    }
  }); function write(data) {
  port.write(data);
  port.drain();
}

function init() {
  write('@ons\n');
  write('@i' + sid + '\n');
  write('@m0768\n');  // 0b01100000000
  write('@m1768\n');  // 0b01100000000
  write('@f0768\n');  // 0b01100000000
  write('@f1512\n');  // 0b01000000000
}

port.on('open', () => {
  init();
  data.forEach((it) => {
    console.log(it);
    write(it+'\n');
  });
});

var parser = new Readline('\n');
port.pipe(parser);

client.on('connect', () => {

  parser.on('data', (data) => {
    data = data.toString();
    let data = data.split(',');
    let sid = data[0];
    let topic;
    if ((sid & 0b00100000000) > 0) {
      topic = sid + '-rx';
    } else {
      topic = sid + '-tx';
    }
    let payload = data[1]; 
    client.publish(topic, data);
  });

});

rl.on('line', (input) => {
  write(input+'\n');
});

