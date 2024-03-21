{
  "variables": {
    "module_name": "modules",
    "module_path": "./lib/binding/",
  },
  "targets": [
    {
      "target_name": "modules",
      "sources": ["modules.cpp"],
      "include_dirs": [
        "lib",
        "lib/nlohmann",
        "lib/boost_1_82_0",
        "lib/gzip-hpp-0.1.0/include",
        "<!(pkg-config --cflags zlib)"
      ],
      "libraries": [
        "<!(pkg-config --libs zlib)"
      ],
      "cflags_cc": [ "-fexceptions", "-std=c++11" ],
      "xcode_settings": {
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
      }
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
