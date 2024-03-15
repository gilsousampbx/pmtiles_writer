export interface GZippeTile {
  buffer: Buffer,
  z: Number,
  x: Number,
  y: Number
}

export interface PMTileGeneratorRespose {
  buffer: Buffer,
  leaf_size: Number
}

export function generate_pmtiles_bundle(
  tiles: GZippeTile[],
  metadata: String
): PMTileGeneratorRespose;
