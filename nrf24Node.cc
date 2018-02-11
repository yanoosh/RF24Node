#include <nan.h>
#include <v8.h>
#include <RF24.h>
#include <RF24Network.h>
#include <iostream>
#include <ctime>
#include <stdio.h>
#include <time.h>
#include <string>
using namespace Nan;
using namespace v8;

RF24 radio(RPI_V2_GPIO_P1_15, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);

NAN_METHOD(Begin){
  if (info.Length() < 2)
      return Nan::ThrowTypeError("Should pass Channel and Node id");

  uint16_t channel = info[0]->Uint32Value();
  uint16_t thisNode = info[1]->Uint32Value();

	radio.begin();
	delay(5);
	network.begin(channel, thisNode);
}

NAN_METHOD(Write){
  if (info.Length() < 3)
      return Nan::ThrowTypeError("Should pass Receiver Node Id, Buffer and buffer size");

  uint16_t otherNode = info[0]->Uint32Value();
  uint16_t size = info[2]->Uint32Value();
  uint8_t * buffer = (uint8_t*)node::Buffer::Data(info[1]->ToObject());

  RF24NetworkHeader header(otherNode);
  bool ok = network.write(header, buffer, size);
  info.GetReturnValue().Set(ok);
}

class ProgressWorker : public AsyncProgressWorker {
 public:
  ProgressWorker(Callback *onMessage, Callback *onFinish, int milliseconds)
    : AsyncProgressWorker(onFinish), progress(onMessage), milliseconds(milliseconds) {}

  ~ProgressWorker() {
    delete progress;
  }

  void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
    int payloadSize = 3;
    char *payload;
  	while(run)
  	{
  		network.update();
  		while (network.available()) {     // Is there anything ready for us?
  			  RF24NetworkHeader header;
          payload = (char *)malloc(payloadSize);
    			network.read(header, payload, payloadSize);
          // //
          // // // payload_pi localPayload;
          // // // localPayload.fromNode = header.from_node;
          // // // strncpy(localPayload.msg, payload.msg, 24);
          progress.Send(reinterpret_cast<const char*>(payload), payloadSize);
  		}
  		delay(milliseconds);
  	}
    // for (int i = 0; i < 10; ++i) {
    //   progress.Send(reinterpret_cast<const char*>(&i), sizeof(int));
    //   delay(milliseconds);
    // }
  }

  void Stop() {
    run = false;
  }

  void HandleProgressCallback(const char *data, size_t count) {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::NewBuffer(const_cast<char*>(data), count).ToLocalChecked()
    };
    progress->Call(1, argv);
  }

 private:
  Callback *progress;
  int milliseconds;
  bool run = true;
};

ProgressWorker *readWorker;

NAN_METHOD(Read){
  const int ON_MESSAGE = 0;
  const int ON_FINISH = 1;
  const int DELAY = 2;
  int delay = 200;

  if (info.Length() < 1) {
    return Nan::ThrowTypeError("Should pass at least 2 arguments: onMessage, onFinish[, delay]");
  }
  if (!info[ON_MESSAGE]->IsFunction()) {
    return Nan::ThrowTypeError("onMessage should be a function");
  }

  if (!info[ON_FINISH]->IsFunction()) {
    return Nan::ThrowTypeError("onFinish should be a function");
  }

  if (info.Length() > 2) {
    if (!info[DELAY]->IsNumber()) {
      return Nan::ThrowTypeError("delay should be a number");
    }

    if (info[DELAY]->Uint32Value() < 200) {
      return Nan::ThrowTypeError("delay should be a least 200");
    }
    delay = info[DELAY]->Uint32Value();
  }

  readWorker = new ProgressWorker(
    new Nan::Callback(info[ON_MESSAGE].As<Function>()),
    new Nan::Callback(info[ON_FINISH].As<Function>()),
    delay
  );

  AsyncQueueWorker(readWorker);
}

NAN_METHOD(PrintDetails) {
  radio.printDetails();
}

NAN_METHOD(Close){
  if (readWorker != NULL) {
    readWorker->Stop();
  }
}


NAN_MODULE_INIT(Init){

    Nan::Set(target, New<String>("begin").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Begin)).ToLocalChecked());

    Nan::Set(target, New<String>("printDetails").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(PrintDetails)).ToLocalChecked());

    Nan::Set(target, New<String>("read").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Read)).ToLocalChecked());

    Nan::Set(target, New<String>("write").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Write)).ToLocalChecked());

    Nan::Set(target, New<String>("close").ToLocalChecked(),
        GetFunction(New<FunctionTemplate>(Close)).ToLocalChecked());
}

NODE_MODULE(nrf24Node, Init)
