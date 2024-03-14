const addon = require('./build/Release/modules');

const z = 0;
const x = 0;
const y = 0;
const buffer = 'my tile';

const response = addon.generate_pmtiles_bundle([{z, x, y, buffer}, {z, x, y, buffer}]);
console.log(response)