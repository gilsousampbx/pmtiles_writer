#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "pmtiles.hpp"
#include "nlohmann/json.hpp"
#include <zlib.h>

using json = nlohmann::json;

struct PMTilesBundlerResponse
{
    std::stringstream buffer;
    uint32_t leaf_size;
};

std::string gzipString(const std::string& str) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, MAX_WBITS | 16, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
        throw(std::runtime_error("deflateInit2 failed while gzipping."));
    }

    zs.next_in = (Bytef*)str.data();
    zs.avail_in = str.size();
    int ret;
    char outbuffer[32768];
    std::string outstring;

    do {
        zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (outstring.size() < zs.total_out) {
            outstring.append(outbuffer, zs.total_out - outstring.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        std::ostringstream oss;
        oss << "Exception during zlib compression: (" << ret << ") " << zs.msg;
        throw(std::runtime_error(oss.str()));
    }

    return outstring;
}

class Writer {
private:
    std::stringstream buffer;
    std::vector<pmtiles::entryv3> tile_entries;
    std::unordered_map<size_t, uint64_t> hash_to_offset;
    std::stringstream tile_stream;
    uint64_t offset = 0;
    uint64_t addressed_tiles = 0;
    bool clustered = true;
    uint8_t minzoom;
    uint8_t maxzoom;
    float minlon;
    float maxlon;
    float minlat;
    float maxlat;

    void updateStatistics(uint8_t z, uint32_t x, uint32_t y) {
        // This is copying the mercantile.bound function
        // @SEE: https://github.com/mapbox/mercantile/blob/5975e1c0e1ec58e99f8e5770c975796e44d96b53/mercantile/__init__.py#L200-L226
        double Z2 { pow(2, z) };
        double x_double = static_cast<double>(x);
        double y_double = static_cast<double>(y);

        double ul_lon_deg = x_double / Z2 * 360.0 - 180.0;
        double ul_lat_rad = atan(sinh(M_PI * (1.0 - 2.0 * y_double / Z2)));
        double ul_lat_deg = ul_lat_rad * M_PI / 180.0;

        double lr_lon_deg = (x + 1.0) / Z2 * 360.0 - 180.0;
        double lr_lat_rad = atan(sinh(M_PI * (1.0 - 2.0 * (y_double + 1.0) / Z2)));
        double lr_lat_deg = lr_lat_rad * M_PI / 180.0 ;

        if (tile_entries.size() == 0 || ul_lon_deg < minlon) {
            minlon = ul_lon_deg;
        }

        if (tile_entries.size() == 0 || ul_lat_deg < minlat) {
            minlat = ul_lat_deg;
        }

        if (tile_entries.size() == 0 || lr_lon_deg > maxlon) {
            maxlon = lr_lon_deg;
        }

        if (tile_entries.size() == 0 || lr_lat_deg > maxlat) {
            maxlat = lr_lat_deg;
        }

        if (tile_entries.size() == 0 || z < minzoom) {
            minzoom = z;
        }
        if (tile_entries.size() == 0 || z > maxzoom) {
            minzoom = z;
        }
    }

    pmtiles::headerv3 initHeader() {
        pmtiles::headerv3 header;

        header.tile_type = pmtiles::TILETYPE_MVT;
        header.tile_compression = pmtiles::COMPRESSION_UNKNOWN;
        header.min_zoom = minzoom;
        header.max_zoom = maxzoom;
        header.min_lon_e7 = static_cast<int32_t>(minlon);
        header.min_lat_e7 = static_cast<int32_t>(minlat);
        header.max_lon_e7 = static_cast<int32_t>(maxlon);
        header.max_lat_e7 = static_cast<int32_t>(maxlat);
        header.center_zoom = (minzoom + maxzoom) / 2;
        header.center_lon_e7 = static_cast<int32_t>((minlon + maxlon) / 2.0);
        header.center_lat_e7 = static_cast<int32_t>((minlat + maxlat) / 2.0);
        header.addressed_tiles_count = addressed_tiles;
        header.tile_entries_count = tile_entries.size();
        header.tile_contents_count = hash_to_offset.size();
        header.clustered = clustered;

        return header;
    }

public:
    Writer() {}

    void write_tile(uint8_t z, uint32_t x, uint32_t y, const std::string& data) {
        updateStatistics(z, x, y);

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

    PMTilesBundlerResponse finalize(const std::string& metadata) {
        pmtiles::headerv3 header { initHeader() };

        std::sort(tile_entries.begin(), tile_entries.end(), pmtiles::entryv3_cmp);

        auto [root_bytes, leaves_bytes, num_leaves] = pmtiles::make_root_leaves(
            [](const std::string& input, uint8_t compression) {
                return gzipString(input.data());
            },
            pmtiles::COMPRESSION_GZIP,
            tile_entries
        );

        header.internal_compression = pmtiles::COMPRESSION_GZIP;
        header.root_dir_offset = 127;
        header.root_dir_bytes = root_bytes.size();
        header.json_metadata_offset = header.root_dir_offset + header.root_dir_bytes;
        header.json_metadata_bytes = metadata.size();
        header.leaf_dirs_offset = header.json_metadata_offset + header.json_metadata_bytes;
        header.leaf_dirs_bytes = leaves_bytes.size();
        header.tile_data_offset = header.leaf_dirs_offset + header.leaf_dirs_bytes;
        header.tile_data_bytes = offset;

        std::string header_bytes = header.serialize();

        PMTilesBundlerResponse response;
        response.buffer << buffer.rdbuf();

        // makes sure the buffer is clear
        response.buffer.str("");
        response.buffer.clear();

        response.buffer.write(header_bytes.c_str(), header_bytes.size());
        response.buffer.write(root_bytes.c_str(), root_bytes.size());
        response.buffer.write(metadata.c_str(), metadata.size());
        response.buffer.write(leaves_bytes.c_str(), leaves_bytes.size());
        response.buffer << tile_stream.rdbuf();

        uint32_t total_leaf_size = header.leaf_dirs_bytes;
        uint32_t num_leaf_nodes = num_leaves;
        uint32_t leaf_size = (num_leaf_nodes > 0) ? total_leaf_size / num_leaf_nodes : 0;
        response.leaf_size = leaf_size;

        return response;
    }
};