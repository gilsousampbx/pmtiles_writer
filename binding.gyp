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
        "GCC_ENABLE_CPP_EXCEPTIONS": "YES"
      }
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "modules" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/modules.node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
