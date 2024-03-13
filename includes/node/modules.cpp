#include <napi.h>
#include "pmtiles.hpp"

void generate_pmtiles_bundle(Napi::CallbackInfo const& info) {
    auto env = info.Env();
    Napi::HandleScope scope(env);
    std::size_t arg_count = info.Length();

    const char* metadata_buffer = nullptr;
    std::size_t metadata_buffer_length = 0;
    std::unique_ptr<std::string> string_ptr;
    Napi::Reference<Napi::Buffer<char>> buffer_ref;

    if (arg_count < 3) {
        Napi::TypeError::New(env, "generate_pmtiles_bundle requires at least 3 arguments").ThrowAsJavaScriptException();
        return;
    }

    auto tiles_arr = info[0].As<Napi::Array>();
    auto z_str = Napi::String::New(env, "z");
    auto x_str = Napi::String::New(env, "x");
    auto y_str = Napi::String::New(env, "y");
    auto buffer_str = Napi::String::New(env, "buffer");
    std::vector<TileInput> tiles;

    if (tiles_arr.Length() == 0) {
        Napi::TypeError::New(env, "The tiles array must have at least one tile").ThrowAsJavaScriptException();
        return;
    }

    for (std::uint32_t i = 0; i < tiles_arr.Length(); ++i) {
        Napi::Value t_item = tiles_arr[i];
        if (!t_item.IsObject()) {
            Napi::TypeError::New(env, "items in the tiles array must be objects with the fields 'z', 'x', 'y' and 'buffer'").ThrowAsJavaScriptException();
            return;
        }

        auto t_obj = t_item.As<Napi::Object>();

        if (!t_obj.Has(z_str) || !t_obj.Has(x_str) || !t_obj.Has(y_str) || !t_obj.Has(buffer_str)) {
            Napi::TypeError::New(env, "items in the tiles array must be objects with the fields 'z', 'x', 'y' and 'buffer'").ThrowAsJavaScriptException();
            return;
        }

        auto z_val = t_obj.Get(z_str);
        auto x_val = t_obj.Get(x_str);
        auto y_val = t_obj.Get(y_str);

        if (!z_val.IsNumber() || !x_val.IsNumber() || !y_val.IsNumber()) {
            Napi::TypeError::New(env, "the 'z', 'x', 'y' values in the tile object must be numbers").ThrowAsJavaScriptException();
            return;
        }

        auto tile_buffer_val = t_obj.Get(buffer_str);

        if (!tile_buffer_val.IsBuffer()) {
            Napi::TypeError::New(env, "the 'buffer' value in the tile object must be a buffer").ThrowAsJavaScriptException();
            return;
        }

        string tile_buffer = tile_buffer_val.As<Napi::Buffer<char>>().Data();
        //auto tile_id = pmtiles::zxy_to_tileid(z_val.As<Napi::Number>().uint8_t(), x: x_val.As<Napi::Number>().uint32_t(), y_val.As<Napi::Number>().uint32_t());
    }

    if (!info[1].IsString() && !info[1].IsBuffer()) {
        Napi::TypeError::New(env, "second argument must be a string or buffer").ThrowAsJavaScriptException();
        return;
    } else if (info[1].IsString()) {
        string_ptr = std::make_unique<std::string>(info[0].As<Napi::String>().Utf8Value());
        metadata_buffer = string_ptr->data();
        metadata_buffer_length = string_ptr->size();
    } else {
        auto napi_buffer = info[1].As<Napi::Buffer<char>>();
        buffer_ref.Reset(napi_buffer, 1);
        metadata_buffer = napi_buffer.Data();
        metadata_buffer_length = napi_buffer.Length();
    }

    // Check third argument, should be a 'callback' function.
    if (!info[2].IsFunction()) {
        Napi::TypeError::New(env, "arg 'callback' must be a function").ThrowAsJavaScriptException();
        return;
    }
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "generate_pmtiles_bundle"), Napi::Function::New(env, generate_pmtiles_bundle));

    return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, init) // NOLINT
