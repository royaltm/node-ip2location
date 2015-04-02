#!/usr/bin/env node
var ben = require('ben')
, Location = require('..')
, assert = require('assert')
, ip2loc = require('ip2location-nodejs')
, dropin = require('../dropin')
   
if (process.argv.length < 3) {
  console.log('usage: node ' + 
    require('path').basename(process.argv[1]) + 
    ' IP2LOCATION-DATABASE.BIN access_mode iterations mask\n\n' +
    '  access_mode: file|cache|mmap|shared|/shared_name');
  process.exit(1);
}

function parseMask(arg) {
  if (arg) {
    if ((/^(?:0x)?\d+$/).test(arg)) {
      return parseInt(arg);
    } else {
      return arg.toUpperCase().split(/[|,+-.]+/).reduce(function(mask, name) {
        if (name in Location) {
          return mask | Location[name];
        } else {
          throw new Error("Unknown mask name: " + name);
        }
      }, 0);
    }
  }
  return Location.ALL;
}

var file = process.argv[2]
  , mode = process.argv[3] || 'file'
  , iter = (process.argv[4]|0) || 10000
  , mask = parseMask(process.argv[5])
  , location = new Location(file, mode)
  ;

var random = Math.random;

function randip() {
  return [random()*100&255,random()*256&255,random()*256&255,random()*256&255].join('.');
}

var ip4buff = new Buffer(4);
function randipbin() {
  ip4buff[0] = random()*100&255;
  ip4buff[1] = random()*256&255;
  ip4buff[2] = random()*256&255;
  ip4buff[3] = random()*256&255;
  return ip4buff;
}

function randipv6() {
  return ["2A04",
          (random()*0x10000|0+0x10000).toString(16).substr(1),
          (random()*0x10000|0+0x10000).toString(16).substr(1),
          (random()*0x10000|0+0x10000).toString(16).substr(1),
          '',
          (random()*0x10000|0+0x10000).toString(16).substr(1)].join(":");
}

var ip6buff = new Buffer(16);
ip6buff.fill(0);
ip6buff[0] = 0x2a;
ip6buff[1] = 0x04;
function randipv6bin() {
  ip6buff.writeUInt16BE(random()*0x10000|0, 2);
  ip6buff.writeUInt16BE(random()*0x10000|0, 4);
  ip6buff.writeUInt16BE(random()*0x10000|0, 6);
  ip6buff.writeUInt16BE(random()*0x10000|0, 14);
  return ip6buff;
}

var hasipv6 = location.ipv6;

function test(testfun, nobin) {
  console.log("ipv4 test x " + iter + " times");
  var ms = ben(iter, function() {
    testfun(randip());
  });

  console.log('query/ms: ' + (1 / ms).toFixed(4));
  console.log('query in: ' + ms + 'ms');

  if (!nobin) {
    console.log("\nipv4bin test x " + iter + " times");
    var ms = ben(iter, function() {
      testfun(randipbin());
    });

    console.log('query/ms: ' + (1 / ms).toFixed(4));
    console.log('query in: ' + ms + 'ms');
  }

  if (hasipv6) {
    console.log("\nipv6 test x " + iter + " times");
    ms = ben(iter, function() {
      testfun(randipv6());
    });

    console.log('query/ms: ' + (1 / ms).toFixed(4));
    console.log('query in: ' + ms + 'ms');

    if (!nobin) {
      console.log("\nipv6bin test x " + iter + " times");
      ms = ben(iter, function() {
        testfun(randipv6bin());
      });

      console.log('query/ms: ' + (1 / ms).toFixed(4));
      console.log('query in: ' + ms + 'ms');
    }
  }
}

console.log(location);
console.log(location.info());

var ip = randip();
console.log(ip);
console.log(location.query(ip, mask));
ip = randipbin();
console.log(ip);
console.log(location.query(ip, mask));

if (location.ipv6) {
  ip = randipv6();
  console.log(ip);
  console.log(location.query(ip, mask));
  ip = randipv6bin();
  console.log(ip);
  console.log(location.query(ip, mask));
}

console.log('-------------------');
console.log("pid: " + process.pid);
console.log("mask: 0x" + (mask|0x100000).toString(16).substr(1));

console.log('-------------------');
console.log("ip2location native:");

test(function(ip) { location.query(ip, mask); });

location.close();

console.log();

dropin.IP2Location_init(file, mode);
console.log('-------------------');
console.log("ip2location dropin:");
console.log("\nIP2Location_get_all()");
test(dropin.IP2Location_get_all);
console.log("\nIP2Location_get_country_short()");
test(dropin.IP2Location_get_country_short);

dropin.IP2Location_init();

console.log();

ip2loc.IP2Location_init(file)
console.log('-------------------');
console.log("ip2location-nodejs:");
console.log("\nIP2Location_get_all()");
test(ip2loc.IP2Location_get_all, true);
console.log("\nIP2Location_get_country_short()");
test(ip2loc.IP2Location_get_country_short.bind(ip2loc), true);
