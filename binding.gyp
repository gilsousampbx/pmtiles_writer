{
  "targets": [
    {
      "target_name": "hello",
      "sources": ["hello.cc"],
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
