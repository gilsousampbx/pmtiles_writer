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

void GeneratePMTilesBundle(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = args.GetIsolate();

    Writer writer;

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
