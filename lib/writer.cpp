#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "pmtiles.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

class Writer {
private:
    std::stringstream buffer;
    std::vector<pmtiles::entryv3> tile_entries;
    std::unordered_map<size_t, uint64_t> hash_to_offset;
    std::stringstream tile_stream;
    uint64_t offset = 0;
    uint64_t addressed_tiles = 0;
    bool clustered = true;

public:
    Writer() {}

    void write_tile(uint8_t z, uint32_t x, uint32_t y, const std::string& data) {
        uint16_t tileid = pmtiles::zxy_to_tileid(z, x, y);

        if (!tile_entries.empty() && tileid < tile_entries.back().tile_id) {
            clustered = false;
        }

        size_t hsh = std::hash<std::string>{}(data);
        if (hash_to_offset.find(hsh) != hash_to_offset.end()) {
            auto last = tile_entries.back();
            auto found = hash_to_offset[hsh];
            if (tileid == last.tile_id + last.run_length && last.offset == found) {
                tile_entries.back().run_length += 1;
            } else {
                tile_entries.emplace_back(tileid, found, data.length(), 1);
            }
        } else {
            tile_stream.write(data.c_str(), data.length());
            tile_entries.emplace_back(tileid, offset, data.length(), 1);
            hash_to_offset[hsh] = offset;
            offset += data.length();
        }

        addressed_tiles += 1;
    }

    std::string finalize(pmtiles::headerv3& header, const json& metadata) {
        header.addressed_tiles_count = addressed_tiles;
        header.tile_entries_count = tile_entries.size();
        header.tile_contents_count = hash_to_offset.size();
        
        std::sort(tile_entries.begin(), tile_entries.end(), pmtiles::entryv3_cmp);

        header.min_zoom = pmtiles::tileid_to_zxy(tile_entries.front().tile_id).z;
        header.max_zoom = pmtiles::tileid_to_zxy(tile_entries.back().tile_id).z;

        auto [root_bytes, leaves_bytes, num_leaves] = pmtiles::make_root_leaves(
            [](const std::string& input, uint8_t compression) {
                return input.data();
            },
            pmtiles::COMPRESSION_NONE,
            tile_entries
        );

        std::string metadata_bytes = metadata.dump();

        header.clustered = clustered;
        header.internal_compression = pmtiles::COMPRESSION_NONE;
        header.root_dir_offset = 127;
        header.root_dir_bytes = root_bytes.size();
        header.json_metadata_offset = header.root_dir_offset + header.root_dir_bytes;
        header.json_metadata_bytes = metadata_bytes.size();
        header.leaf_dirs_offset = header.json_metadata_offset + header.json_metadata_bytes;
        header.leaf_dirs_bytes = leaves_bytes.size();
        header.tile_data_offset = header.leaf_dirs_offset + header.leaf_dirs_bytes;
        header.tile_data_bytes = offset;

        std::string header_bytes = header.serialize();

        buffer.write(header_bytes.c_str(), header_bytes.size());
        buffer.write(root_bytes.c_str(), root_bytes.size());
        buffer.write(metadata_bytes.c_str(), metadata_bytes.size());
        buffer.write(leaves_bytes.c_str(), leaves_bytes.size());
        buffer << tile_stream.rdbuf();

        return buffer.str();
    }
};