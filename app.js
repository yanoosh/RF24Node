var rf24 = require('./index.js');

rf24.begin(90, 0);
rf24.printDetails();
var i = 0;

// setInterval(function() {
//   const write = Buffer.from([i++, 1, 2])
//   console.log("Send: ", write)
//   rf24.write(1, write, write.length)
// }, 2000)

// while(true){
//     rf24.update();
//     var status = rf24.available();
//     if(status==true){
//         var data = rf24.read();
//         console.log(data);
//     }
// }

// rf24.begin(90,00);
// rf24.printDetails();
// rf24.write(1,"Ack");
//
const onMessage = function(data){
    // console.log("from: ", from);
    // console.log(data);
    // rf24.write(1,"Ack");
    console.log(
      "from", data.readInt32LE(0),
      "motionSensor", data.readInt32LE(4),
      "humidity", data.readFloatLE(8),
      "temp", data.readFloatLE(12)
    )
}

const onFinish = () => {
  console.log("listening finished")
}

rf24.read(onMessage, onFinish, 64, 1000);
console.log('start listening')
//
// setInterval(function() {
//   console.log("working!")
// }, 1000)
//
process.on('SIGINT', exitHandler);

function exitHandler() {
  rf24.close();
  process.exit();
}
