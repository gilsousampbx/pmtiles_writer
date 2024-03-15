const addon = require('./build/Release/modules');

const z = 0;
const x = 0;
const y = 0;
const buffer = 'my tile';
const vector_layers = [
  {"id": "layer1", "fields": {"hello": "string", "population": "number", "hola": "boolean"}, "minzoom": 0, "maxzoom": 3},
  {"id": "layer2", "fields": {"hoi": "string"}, "minzoom": 5, "maxzoom": 5},
];
const metadata = {
  "name": "user.tileset_name",
  "vector_layers": vector_layers,
  "sharding_scheme_version": "v1",
};

const response = addon.generate_pmtiles_bundle([{ z, x, y, buffer }, { z, x, y, buffer }], JSON.stringify(metadata));
console.log(response);
