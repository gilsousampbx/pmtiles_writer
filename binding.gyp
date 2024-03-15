{
  "targets": [
    {
      "target_name": "modules",
      "sources": ["modules.cpp"],
      "include_dirs": [
        "libs",
        "libs/nlohmann",
        "<!(pkg-config --cflags zlib)"
      ],
      "libraries": [
        "<!(pkg-config --libs zlib)"
      ],
      "cflags_cc": [ "-fexceptions", "-std=c++11" ],
      "xcode_settings": {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
      }
    }
  ]
}
