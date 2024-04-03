#include <node.h>
#include "lib/pmtiles.hpp"
#include "lib/writer.cpp"
#include <iostream>

#include <node_buffer.h>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;
using v8::Exception;

void GeneratePMTilesBundle(const FunctionCallbackInfo<Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();

    if (!args[0]->IsArray()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Argument must be an array").ToLocalChecked()));
        return;
    }

    Local<Array> tiles_arr = Local<Array>::Cast(args[0]);

    Writer writer;

    for (size_t i = 0; i < tiles_arr->Length(); ++i) {
        Local<Value> t_item = tiles_arr->Get(isolate->GetCurrentContext(), i).ToLocalChecked();

        if (!t_item->IsObject()) {
            isolate->ThrowException(v8::Exception::TypeError(
                String::NewFromUtf8(isolate, "Items in the tiles array must be objects with the fields 'z', 'x', 'y', and 'buffer'").ToLocalChecked()));
            return;
        }

        Local<Object> t_obj = t_item->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

        int z = t_obj->Get(
            isolate->GetCurrentContext(),
            String::NewFromUtf8(isolate, "z").ToLocalChecked()
        ).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();

        int x = t_obj->Get(
            isolate->GetCurrentContext(),
            String::NewFromUtf8(isolate, "x").ToLocalChecked()
        ).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();

        int y = t_obj->Get(
            isolate->GetCurrentContext(),
            String::NewFromUtf8(isolate, "y").ToLocalChecked()
        ).ToLocalChecked()->Int32Value(isolate->GetCurrentContext()).ToChecked();

        v8::Local<v8::Value> bufferValue = t_obj->Get(
            isolate->GetCurrentContext(),
            v8::String::NewFromUtf8(isolate, "buffer").ToLocalChecked()
        ).ToLocalChecked();

        v8::Local<v8::Object> bufferObj = bufferValue->ToObject(isolate->GetCurrentContext()).ToLocalChecked();

        std::stringstream ss;

        size_t bufferLength = node::Buffer::Length(bufferObj);
        char* bufferData = node::Buffer::Data(bufferObj);

        ss.write(bufferData, bufferLength);

        writer.write_tile(z, x, y, ss);
    }

    if (!args[1]->IsString()) {
        isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, "Argument must be a string").ToLocalChecked()));
        return;
    }

    std::string metadata = *String::Utf8Value(isolate, args[1]->ToString(isolate->GetCurrentContext()).ToLocalChecked());

    PMTilesBundlerResponse response = writer.finalize(metadata);

    Local<String> bufferString = String::NewFromOneByte(
        isolate,
        reinterpret_cast<const uint8_t*>(response.buffer.str().c_str()),
        v8::NewStringType::kNormal,
        response.buffer.str().length()
    ).ToLocalChecked();

    Local<Object> responseObj = v8::Object::New(isolate);

    responseObj->Set(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "leaf_size", v8::NewStringType::kNormal).ToLocalChecked(), v8::Integer::New(isolate, response.leaf_size)).FromJust();

    responseObj->Set(
        isolate->GetCurrentContext(),
        String::NewFromUtf8(isolate, "buffer").ToLocalChecked(),
        bufferString
    );

    args.GetReturnValue().Set(responseObj);
}

void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "generate_pmtiles_bundle", GeneratePMTilesBundle);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
