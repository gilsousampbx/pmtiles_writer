export interface GZippedTile {
  buffer: Buffer,
  z: Number,
  x: Number,
  y: Number
}

export interface PMTileGeneratorResponse {
  buffer: Buffer,
  leaf_size: Number
}

export function generate_pmtiles_bundle(
  tiles: GZippedTile[],
  metadata: String
): PMTileGeneratorResponse;
