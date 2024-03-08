#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <gzip/compress.hpp>
#include <nlohmann/json.hpp>
#include "pmtiles.hpp"

using json = nlohmann::json;

class Writer {
private:
    std::ofstream file;
    std::vector<pmtiles::entryv3> tile_entries;
    std::unordered_map<size_t, uint64_t> hash_to_offset;
    std::stringstream tile_stream;
    uint64_t offset = 0;
    uint64_t addressed_tiles = 0;
    bool clustered = true;

public:
    Writer(const std::string& filename) : file(filename, std::ios::binary) {
        if (!file.is_open()) {
            throw std::runtime_error("Failed to open file");
        }
    }

    void write_tile(uint64_t tileid, const std::string& data) {
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

    void finalize(const pmtiles::headerv3& header, const json& metadata) {
        header.addressed_tiles_count = addressed_tiles;
        header.tile_entries_count = tile_entries.size();
        header.tile_contents_count = hash_to_offset.size();

        std::sort(tile_entries.begin(), tile_entries.end(), pmtiles::entryv3_cmp);

        header.min_zoom = pmtiles::tileid_to_zxy(tile_entries.front().tile_id).z;
        header.max_zoom = pmtiles::tileid_to_zxy(tile_entries.back().tile_id).z;

        auto [root_bytes, leaves_bytes, num_leaves] = pmtiles::make_root_leaves(
            [](const std::string& input, uint8_t compression) {
                return gzip::compress(input.data(), input.size());
                
                return input;
            },
            pmtiles::COMPRESSION_UNKNOWN, // change the compression to unknow
            tile_entries
        );

        auto compressed_metadata = gzip::compress(metadata.dump().data(), metadata.dump().size());

        header.clustered = clustered;
        header.internal_compression = pmtiles::COMPRESSION_UNKNOWN; // change to compression unknow
        header.root_dir_offset = 127;
        header.root_dir_bytes = root_bytes.size();
        header.json_metadata_offset = header.root_dir_offset + header.root_dir_bytes;
        header.json_metadata_bytes = compressed_metadata.size();
        header.leaf_dirs_offset = header.json_metadata_offset + header.json_metadata_bytes;
        header.leaf_dirs_bytes = leaves_bytes.size();
        header.tile_data_offset = header.leaf_dirs_offset + header.leaf_dirs_bytes;
        header.tile_data_bytes = offset;

        std::string header_bytes = header.serialize();

        // string first pointer
        file.write(header_bytes.c_str(), header_bytes.size());
        file.write(root_bytes.c_str(), root_bytes.size());
        file.write(compressed_metadata.c_str(), compressed_metadata.size());
        file.write(leaves_bytes.c_str(), leaves_bytes.size());
        file << tile_stream.rdbuf();
    }
};


int main() {

    std::string tile_data = "This is tile data";

    pmtiles::headerv3 header;
    header.tile_type = pmtiles::TILETYPE_PNG;
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

    Writer writer("stamen_toner_maxzoom3.pmtiles");
    writer.write_tile(pmtiles::zxy_to_tileid(0, 0, 0), tile_data);
    writer.write_tile(pmtiles::zxy_to_tileid(1, 0, 0), tile_data);
    writer.finalize(header, metadata);

    std::cout << pmtiles::zxy_to_tileid(3, 0, 0) << std::endl;

    return 0;
}