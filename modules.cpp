#include <node.h>
#include "lib/pmtiles.hpp"
#include "lib/nlohmann/json.hpp"
#include "lib/writer.cpp"

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Exception;
using v8::Array;
using v8::Context;
using v8::Boolean;

void GeneratePMTilesBundle(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();
    auto context = isolate->GetCurrentContext();

    if (args.Length() < 2) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "generate_pmtiles_bundle requires at least 2 arguments").ToLocalChecked()));
        return;
    }

    auto tiles_arr = Local<Array>::Cast(args[0]);
    auto z_str = String::NewFromUtf8(isolate, "z").ToLocalChecked();
    auto x_str = String::NewFromUtf8(isolate, "x");
    auto y_str = String::NewFromUtf8(isolate, "y");
    auto buffer_str = String::NewFromUtf8(isolate, "buffer");

    if (tiles_arr->Length() == 0) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "The tiles array must have at least one tile").ToLocalChecked()));
        return;
    }

    Writer writer;

    for (size_t i = 0; i < tiles_arr->Length(); ++i) {
        auto t_item = tiles_arr->Get(context, i).ToLocalChecked();

        if (!t_item->IsObject()) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "items in the tiles array must be objects with the fields 'z', 'x', 'y' and 'buffer'").ToLocalChecked()));
            return;
        }

        auto t_obj = Local<Object>::Cast(t_item);
        if (t_obj->Has(context, z_str).ToChecked() == Local<Boolean>::True()) {
            isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate, "items in the tiles array must be objects with the fields 'z', 'x', 'y' and 'buffer'").ToLocalChecked()));
            return;
        }
    }

    pmtiles::headerv3 header;
    header.tile_type = pmtiles::TILETYPE_MVT;
    header.tile_compression = pmtiles::COMPRESSION_UNKNOWN;
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

    writer.write_tile(0, 0, 0, "my tile 1");
    writer.write_tile(1, 0, 0, "my tile 2");
    writer.write_tile(2, 0, 0, "my tile 3");

    writer.finalize(header, metadata);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, metadata.dump().data()).ToLocalChecked());
}

void Initialize(Local<Object> exports) {
    NODE_SET_METHOD(exports, "generate_pmtiles_bundle", GeneratePMTilesBundle);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize)
