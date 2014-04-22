Location = require('./index')
if (process.argv.length < 3) {
  console.log('usage: node ' + 
    require('path').basename(process.argv[1]) + 
    ' IP2LOCATION-DATABASE.BIN access_mode iterations');
  console.log();
  console.log('  access_mode: file|cache|mmap|shared|/shared_name');
  process.exit(1);
}
var file = process.argv[2]
  , mode = process.argv[3]
  , iter = (process.argv[4]|0) || 100000
  , location = new Location(file, mode)
;
var random = Math.random;
function randip() {
  return [random()*256&255,random()*256&255,random()*256&255,random()*256&255].join('.');
}
var type = Location.COUNTRYSHORT|Location.REGION|Location.CITY|Location.LATITUDE|Location.LONGITUDE;

console.log(location);
console.log(location.info());
console.log(location.query(randip(), type));
console.log('pid: ' + process.pid);
var time = Date.now();
for (var i = 0; i < iter; i++) location.query(randip(), type);
time = (Date.now() - time) / 1000.0;
console.log(iter / time + ' / s.');
console.log('avg: ' + time / iter + ' s.');
