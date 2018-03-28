'use strict';

process.on('unhandledRejection', console.dir);

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
    },err => {
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

parser.on('data', (data) => {
    data = data.toString();
    console.log(data);
});

rl.on('line', (input) => {
    write(input+'\n');
});

