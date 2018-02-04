var rf24 = require('./build/Release/nrf24Node.node');

// rf24.beginRadio();
// rf24.beginNetwork(90,00);
// rf24.printDetails();
// while(true){
//     rf24.update();
//     var status = rf24.available();
//     if(status==true){
//         var data = rf24.read();
//         console.log(data);
//     }
// }

rf24.begin(90,00);
rf24.printDetails();
rf24.write(1,"Ack");

const onMessage = function(from, data){
    console.log("resonse", from);
    // console.log(data);
    // rf24.write(1,"Ack");
}

const onFinish = () => {
  console.log("listening finished")
}

rf24.readAsync(onMessage, onFinish, 1000.1);
console.log('start listening')

process.on('SIGINT', exitHandler);

function exitHandler() {
    process.exit();
    rf24.write(1,"Parent ending");
    rf24.close();
}
