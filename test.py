from pmtiles.writer import Writer
import os
import mercantile

from pmtiles.tile import tileid_to_zxy, Compression, TileType

PMTILES_COMBINED_HEADER_ROOT_DIR_SIZE = 16384
PMTILES_HEADER_SIZE = 127

# Header fields
MIN_ZOOM = "min_zoom"
MAX_ZOOM = "max_zoom"
CENTER_ZOOM = "center_zoom"
MIN_LON_E7 = "min_lon_e7"
MAX_LON_E7 = "max_lon_e7"
CENTER_LON_E7 = "center_lon_e7"
MIN_LAT_E7 = "min_lat_e7"
MAX_LAT_E7 = "max_lat_e7"
CENTER_LAT_E7 = "center_lat_e7"
TILE_TYPE = "tile_type"
TILE_COMPRESSION = "tile_compression"
ADDRESSED_TILES_COUNT = "addressed_tiles_count"
TILE_ENTRIES_COUNT = "tile_entries_count"
TILE_CONTENTS_COUNT = "tile_contents_count"
CLUSTERED = "clustered"
INTERNAL_COMPRESSION = "internal_compression"
ROOT_OFFSET = "root_offset"
ROOT_LENGTH = "root_length"
METADATA_OFFSET = "metadata_offset"
METADATA_LENGTH = "metadata_length"
LEAF_DIRECTORY_OFFSET = "leaf_directory_offset"
LEAF_DIRECTORY_LENGTH = "leaf_directory_length"
TILE_DATA_OFFSET = "tile_data_offset"
TILE_DATA_LENGTH = "tile_data_length"

def create_header(tileids):
    """
    From https://github.com/protomaps/PMTiles/blob/464221eaa4074784343152feb7b554663e0e2661/python/pmtiles/convert.py#L11
    """

    minzoom = 100
    maxzoom = -1
    minlon = 180
    maxlon = -180
    minlat = 90
    maxlat = -90

    for tileid in tileids:
        z, x, y = tileid_to_zxy(tileid)
        minzoom = min(z, minzoom)
        maxzoom = max(z, maxzoom)

        bound = mercantile.bounds(x, y, z)
        minlon = min(bound.west, minlon)
        minlat = min(bound.south, minlat)
        maxlon = max(bound.east, maxlon)
        maxlat = max(bound.north, maxlat)

    header = {
        MIN_ZOOM: minzoom,
        MAX_ZOOM: maxzoom,
        CENTER_ZOOM: int((minzoom + maxzoom) / 2),
        MIN_LON_E7: int(minlon * 10000000),
        MAX_LON_E7: int(maxlon * 10000000),
        CENTER_LON_E7: int((minlon + maxlon) / 2 * 10000000),
        MIN_LAT_E7: int(minlat * 10000000),
        MAX_LAT_E7: int(maxlat * 10000000),
        CENTER_LAT_E7: int((minlat + maxlat) / 2 * 10000000),
        TILE_TYPE: TileType.MVT,
        # Because the off-the-shelf pmtiles reader decompresses a tile if it knows
        # the tile is compressed, but api-vectortiles need to return gzipped tiles,
        # below we are lying that we don't know what the tile compression is
        # although we do know that it is gzip.
        # (see https://github.com/mapbox/core-tiles/pull/108#discussion_r1224139442)
        TILE_COMPRESSION: Compression.UNKNOWN,
    }

    return header

if __name__ == "__main__":
    print(os.getcwd())

    with open('test-output.pmtiles', 'wb') as output:
        writer = Writer(output)

        with open("0-0-0.mvt", "rb") as f:
            writer.write_tile(0, f.read())

        with open("1-0-0.mvt", "rb") as f:
            writer.write_tile(1, f.read())

        with open("1-0-1.mvt", "rb") as f:
            writer.write_tile(2, f.read())

        with open("1-1-0.mvt", "rb") as f:
            writer.write_tile(3, f.read())

        with open("1-1-1.mvt", "rb") as f:
            writer.write_tile(4, f.read())

        my_entires = [item.tile_id for item in writer.tile_entries]

        header = create_header(my_entires)
        metadata = {
            'name': 'user.tileset_name',
            'vector_layers': [
                {"id": "layer1", "fields": {"hello": "string", "population": "number", "hola": "boolean"}, "minzoom": 0, "maxzoom": 1},
                {"id": "layer2", "fields": {"hoi": "string"}, "minzoom": 0, "maxzoom": 1},
            ],
            'sharding_scheme_version': 'single_shard'
        }
        writer.finalize(header, metadata)

        # writer.tile_f.seek(0)

        # output.write(writer.header_bytes)
        # output.write(writer.root_bytes)
        # output.write(writer.leaves_bytes)
        # output.write(writer.compressed_metadata)
        # output.write(writer.tile_f.read())
        # writer.close()