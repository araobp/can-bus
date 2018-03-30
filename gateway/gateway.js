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
    f: false,
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
  });

function write(data) {
  port.write(data);
  port.drain();
}

function init() {
  write('@ons\n');
  write('@i' + sid + '\n');
  if (args.f) {
    write('@m0' + 0b01100000000 + '\n');
    write('@m1' + 0b01100000000 + '\n');
    write('@f0' + 0b01100000000 + '\n');
    write('@f1' + 0b01000000000 + '\n');
  } else {
    write('@m00\n');
    write('@m10\n');
    write('@f00\n');
    write('@f10\n');
    write('@f20\n');
    write('@f30\n');
    write('@f40\n');
    write('@f50\n');
  }
}

port.on('open', () => {
  init();
});

var parser = new Readline('\n');
port.pipe(parser);

function timestamp() {
  return new Date().getTime();
}

client.on('connect', () => {

  client.subscribe('+/tx');
  client.subscribe('+/rx');

  parser.on('data', (data) => {
    data = data.toString().split(',');
    let sid = data[0];
    let topic;
    if ((sid & 0b00100000000) > 0) {
      topic = sid + '/rx';
    } else {
      topic = sid + '/tx';
    }
    let payload = JSON.stringify({
      timestamp: timestamp(),
      data: data[1]
    });
    client.publish(topic, payload);
    console.log('topic: ' + topic + ', payload: ' + payload);
  });

});

rl.on('line', (input) => {
  write(input+'\n');
});

client.on('message', (topic, message) => {
  console.log('topic: ' + topic + ', message: ' + message);
  message = JSON.parse(message);
  topic = topic.split('/');
  let txrx = topic[1];
  let sid;
  if (txrx == 'tx') {
    sid = topic[0];
  } else if (txrx == 'rx') {
    sid = (Number(topic[0]) | 0b00100000000).toString();
  }
  write('@i' + sid + '\n');
  write(message.data + '\n');
});

