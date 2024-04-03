export interface GZippedTile {
  buffer: Buffer,
  z: number,
  x: number,
  y: number
}

export interface PMTileGeneratorResponse {
  buffer: string,
  leaf_size: number
}

export function generate_pmtiles_bundle(
  tiles: GZippedTile[],
  metadata: string
): PMTileGeneratorResponse;
