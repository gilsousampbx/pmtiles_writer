{
  "targets": [
    {
      "target_name": "modules",
      "sources": ["modules.cpp"],
      "include_dirs": [
        "libs",
        "libs/nlohmann"
      ],
      "cflags_cc": [ "-fexceptions", "-std=c++11" ],
      "xcode_settings": {
        'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
      }
    }
  ]
}
