#!/usr/bin/env node
var ben = require('ben')
, Location = require('..')
, assert = require('assert')
, ip2loc = require("ip2location-nodejs");
   
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
  , mode = process.argv[3]
  , iter = (process.argv[4]|0) || 10000
  , mask = parseMask(process.argv[5])
  , location = new Location(file, mode)
  , IP2Location_get_all = ip2loc.IP2Location_get_all
  ;

var random = Math.random;

function randip() {
  return [random()*100&255,random()*256&255,random()*256&255,random()*256&255].join('.');
}

function randipv6() {
  return ["2A04",
          (random()*0x10000|0+0x10000).toString(16).substr(1),
          (random()*0x10000|0+0x10000).toString(16).substr(1),
          (random()*0x10000|0+0x10000).toString(16).substr(1),
          '',
          (random()*0x10000|0+0x10000).toString(16).substr(1)].join(":");
}

//461254.6125461254 C R C
//397298.3710766786 C R C S
//270270.2702702703 ALL
//45516.61356395084 ALL


//cache
//504032.2580645161 C R C
//444444.4444444444 C R C S
//373552.4841240194 ALL
//file
//50175.61465127947 C R C S
//47732.6968973747 ALL

function test(testfun) {
  console.log("\nipv4 test x " + iter + " times");
  var ms = ben(iter, function() {
    testfun(randip());
  });

  console.log('query/ms: ' + (1 / ms).toFixed(4));
  console.log('query in: ' + ms + 'ms');

  if (location.ipv6) {
    console.log("\nipv6 test x " + iter + " times");
    ms = ben(iter, function() {
      testfun(randipv6());
    });

    console.log('query/ms: ' + (1 / ms).toFixed(4));
    console.log('query in: ' + ms + 'ms');
  }
}

console.log(location);
console.log(location.info());

var ip = randip();
console.log(ip);
console.log(location.query(ip, mask));

if (location.ipv6) {
  ip = randipv6();
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

ip2loc.IP2Location_init(file)

console.log();
console.log('-------------------');
console.log("ip2location-nodejs:");

test(IP2Location_get_all);
