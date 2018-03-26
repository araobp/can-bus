const SerialPort = require('serialport');
const Readline = SerialPort.parsers.Readline;

const SID_MAX = 2047;

const PORT = '/dev/ttyUSB0';

const port = new SerialPort(PORT, {
            baudRate: 9600
    },err => {
        if (err) {
            return console.log('Error:', err.message);
    }
});
const parser = new Readline('\n');
port.pipe(parser);

function write(port, data) {
    port.write(data);
    port.drain();
}

port.on('open', () => {
    write(port, '@ols\n');
    write(port, '@i' + SID_MAX + '\n');
    // Loopback test: half of them would be discarded
    for(var i=0;i<100;i++) {
        write(port, i.toString()+'abc'+i.toString()+'\n');
    }
    for(var i=100;i<200;i++) {
        write(port, i.toString()+'de'+i.toString());
    }
});

parser.on('data', (data) => {
    data = data.toString();
    console.log(data);
});

