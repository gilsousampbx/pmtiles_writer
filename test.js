var binary = require('@mapbox/node-pre-gyp');
var path = require('path');
var binding_path = binary.find(path.resolve(path.join(__dirname,'./package.json')));
var binding = require(binding_path);
var fs = require('fs');



const z = 0;
const x = 0;
const y = 0;
const buffer = 'my tile';
const vector_layers = [
  {"id": "layer1", "fields": {"hello": "string", "population": "number", "hola": "boolean"}, "minzoom": 0, "maxzoom": 1},
  {"id": "layer2", "fields": {"hoi": "string"}, "minzoom": 0, "maxzoom": 1},
];
const metadata = {
  "name": "user.tileset_name",
  "vector_layers": vector_layers,
  "sharding_scheme_version": "single_shard",
};

const tiles = [
  {
    buffer: fs.readFileSync(`${__dirname}/0-0-0.mvt`),
    z: 0,
    x: 0,
    y: 0,
  },
  {
    buffer: fs.readFileSync(`${__dirname}/1-0-0.mvt`),
    z: 1,
    x: 0,
    y: 0,
  },
  {
    buffer: fs.readFileSync(`${__dirname}/1-1-0.mvt`),
    z: 1,
    x: 1,
    y: 0,
  },
  {
    buffer: fs.readFileSync(`${__dirname}/1-0-1.mvt`),
    z: 1,
    x: 0,
    y: 1,
  },
  {
    buffer: fs.readFileSync(`${__dirname}/1-1-1.mvt`),
    z: 1,
    x: 1,
    y: 1,
  },
];

const response = binding.generate_pmtiles_bundle(tiles, JSON.stringify(metadata));
fs.writeFileSync(`${__dirname}/bundle.pmtiles`, response.buffer);
console.log(response);
