#include <node.h>
#include "lib/pmtiles.hpp"
#include "lib/nlohmann/json.hpp"
#include "lib/writer.cpp"
#include <iostream>

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

void GeneratePMTilesBundle(const FunctionCallbackInfo<Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();

    if (!args[0]->IsArray()) {
        isolate->ThrowException(v8::Exception::TypeError(
            v8::String::NewFromUtf8(isolate, "Argument must be an array").ToLocalChecked()));
        return;
    }

    v8::Local<v8::Array> tiles_arr = v8::Local<v8::Array>::Cast(args[0]);

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

        std::string tile = *String::Utf8Value(
            isolate, 
            t_obj->Get(isolate->GetCurrentContext(), String::NewFromUtf8(isolate, "buffer").ToLocalChecked()).ToLocalChecked()
        );

        writer.write_tile(z, x, y, tile);
        std::cout << tile << std::endl;
    }

    pmtiles::headerv3 header;
    header.tile_type = pmtiles::TILETYPE_UNKNOWN;
    header.tile_compression = pmtiles::COMPRESSION_NONE;
    header.min_zoom = 0;
    header.max_zoom = 3;
    header.min_lon_e7 = static_cast<int32_t>(-180.0 * 10000000);
    header.min_lat_e7 = static_cast<int32_t>(-85.0 * 10000000);
    header.max_lon_e7 = static_cast<int32_t>(180.0 * 10000000);
    header.max_lat_e7 = static_cast<int32_t>(85.0 * 10000000);
    header.center_zoom = 0;
    header.center_lon_e7 = 0;
    header.center_lat_e7 = 0;

    json metadata = {
        {"attribution", "Map tiles by <a href='http://stamen.com'>Stamen Design</a>, under <a href='http://creativecommons.org/licenses/by/3.0'>CC BY 3.0</a>. Data by <a href='http://openstreetmap.org'>OpenStreetMap</a>, under <a href='http://www.openstreetmap.org/copyright'>ODbL</a>."},
    };

    PMTilesBundlerResponse response = writer.finalize(header, metadata);

    Local<String> bufferString = String::NewFromUtf8(
        isolate,
        response.buffer.str().c_str(),
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
