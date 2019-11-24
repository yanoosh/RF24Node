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

RF24 radio(RPI_V2_GPIO_P1_22, BCM2835_SPI_CS0, BCM2835_SPI_SPEED_8MHZ);
RF24Network network(radio);

NAN_METHOD(Begin){
  if (info.Length() < 2)
      return Nan::ThrowTypeError("Should pass Channel and Node id");

  uint16_t channel = info[0]->Uint32Value();
  uint16_t thisNode = info[1]->Uint32Value();

	radio.begin();
	radio.setDataRate(RF24_250KBPS);
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
  ProgressWorker(Callback *onMessage, Callback *onFinish, int payloadSize, int milliseconds)
    : AsyncProgressWorker(onFinish), progress(onMessage), payloadSize(payloadSize), milliseconds(milliseconds) {}

  ~ProgressWorker() {
    delete progress;
  }

  void Execute (const AsyncProgressWorker::ExecutionProgress& progress) {
    char * payload = (char *)malloc(payloadSize);
  	while(run)
  	{
  		network.update();
  		while (network.available()) {     // Is there anything ready for us?
  			  RF24NetworkHeader header;
          memset(payload, 0, payloadSize);
    			network.read(header, payload, payloadSize);
          progress.Send(reinterpret_cast<const char*>(payload), payloadSize);
  		}
  		delay(milliseconds);
  	}
    free((void *)payload);
  }

  void Stop() {
    run = false;
  }

  void HandleProgressCallback(const char *data, size_t count) {
    Nan::HandleScope scope;

    v8::Local<v8::Value> argv[] = {
        Nan::CopyBuffer(const_cast<char*>(data), count).ToLocalChecked()
    };
    progress->Call(1, argv);
  }

 private:
  Callback *progress;
  int payloadSize;
  int milliseconds;
  bool run = true;
};

ProgressWorker *readWorker;

NAN_METHOD(Read){
  const int ON_MESSAGE = 0;
  const int ON_FINISH = 1;
  const int PAYLOAD_SIZE = 2;
  const int DELAY = 3;
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

  if (!info[PAYLOAD_SIZE]->IsNumber()) {
    return Nan::ThrowTypeError("payload size should be a number");
  }

  if (info[PAYLOAD_SIZE]->Uint32Value() < 1) {
    return Nan::ThrowTypeError("payload size should be at least 1");
  }

  if (info.Length() > 3) {
    if (!info[DELAY]->IsNumber()) {
      return Nan::ThrowTypeError("delay should be a number");
    }

    if (info[DELAY]->Uint32Value() < 200) {
      return Nan::ThrowTypeError("delay should be at least 200");
    }
    delay = info[DELAY]->Uint32Value();
  }

  readWorker = new ProgressWorker(
    new Nan::Callback(info[ON_MESSAGE].As<Function>()),
    new Nan::Callback(info[ON_FINISH].As<Function>()),
    info[PAYLOAD_SIZE]->Uint32Value(),
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
