var test = require("tap").test
  , Location = require('..')
  , fs = require('fs')
  , path = require('path')
  , IP4DBNAME = path.join(__dirname, '..',
                  fs.readdirSync(path.join(__dirname, '..')).
                    filter(function(n) { return (/^IP-.*\.BIN$/).test(n); })[0])
  , IP6DBNAME = path.join(__dirname, '..',
                  fs.readdirSync(path.join(__dirname, '..')).
                  filter(function(n) { return (/^IPV6-.*\.BIN$/).test(n); })[0])
  ;

var googleResult = {
    country_short: 'US',
    country_long: 'United States',
    region: 'California',
    city: 'Mountain View',
    isp: 'Google Inc.',
    latitude: 37.40599060058594,
    longitude: -122.0785140991211,
    domain: 'google.com',
    zipcode: '94043',
    timezone: '-08:00',
    netspeed: 'T1',
    iddcode: '1',
    areacode: '650',
    weatherstationcode: 'USCA0746',
    weatherstationname: 'Mountain View',
    mcc: '-',
    mnc: '-',
    mobilebrand: '-',
    elevation: '31',
    usagetype: 'SES'
  };

test("should be a function", function(t) {
  t.type(Location, 'function');
  t.end();
});

test("should have numeric constants", function(t) {
  t.strictEqual(Location.COUNTRY_SHORT,      1);
  t.strictEqual(Location.COUNTRY_LONG,       2);
  t.strictEqual(Location.REGION,             4);
  t.strictEqual(Location.CITY,               8);
  t.strictEqual(Location.ISP,                16);
  t.strictEqual(Location.LATITUDE,           32);
  t.strictEqual(Location.LONGITUDE,          64);
  t.strictEqual(Location.DOMAIN,             128);
  t.strictEqual(Location.ZIPCODE,            256);
  t.strictEqual(Location.TIMEZONE,           512);
  t.strictEqual(Location.NETSPEED,           1024);
  t.strictEqual(Location.IDDCODE,            2048);
  t.strictEqual(Location.AREACODE,           4096);
  t.strictEqual(Location.WEATHERSTATIONCODE, 8192);
  t.strictEqual(Location.WEATHERSTATIONNAME, 16384);
  t.strictEqual(Location.MCC,                32768);
  t.strictEqual(Location.MNC,                65536);
  t.strictEqual(Location.MOBILEBRAND,        131072);
  t.strictEqual(Location.ELEVATION,          262144);
  t.strictEqual(Location.USAGETYPE,          524288);
  t.strictEqual(Location.ALL,                1048575);
  t.end();
});

test("should open and close ip2location databases", function(t) {
  var location = new Location(IP4DBNAME);
  t.type(location, Location);
  t.strictEqual(location.mode, "file");
  t.strictEqual(location.opened, true);
  t.strictEqual(location.ipv6, false);
  location.close();
  t.strictEqual(location.opened, false);
  t.strictEqual(location.mode, "closed");
  t.strictEqual(location.ipv6, false);

  location = new Location(IP6DBNAME);
  t.type(location, Location);
  t.strictEqual(location.mode, "file");
  t.strictEqual(location.opened, true);
  t.strictEqual(location.ipv6, true);
  location.close();
  t.strictEqual(location.opened, false);
  t.strictEqual(location.mode, "closed");
  t.strictEqual(location.ipv6, false);
  t.end();
});

test("should open ip2location databases with different modes", function(t) {
  [
    "file",
    "cache",
    "mmap",
    "shared",
    "/___IP2LocationTestName.shared"
  ].forEach(function(mode) {
    var location = new Location(IP4DBNAME, mode);
    t.type(location, Location);
    t.strictEqual(location.opened, true);
    t.strictEqual(location.ipv6, false);
    var copybythisprocess = false;
    var sharedname = mode;
    var info = location.info();
    switch(mode) {
      case "file":
        break;
      case "shared":
        sharedname = '/IP2location_Shm';
      case "cache":
        copybythisprocess = true;
        sharedname == mode && (sharedname = void(0));
      case "mmap":
        sharedname == mode && (sharedname = void(0));
        t.ok(info.cachesize >= info.filesize, info.cachesize + " < " + info.filesize);
      default:
        if (sharedname == mode) {
          t.strictEqual(location.mode, "shared");
          copybythisprocess = true;
        } else {
          t.strictEqual(location.mode, mode);
        }
        t.strictEqual(info.sharedname, sharedname);
        t.strictEqual(info.copybythisprocess, copybythisprocess);
        t.strictEqual(info.cacheoccupants, 1);
        var location2 = new Location(IP4DBNAME, mode);
        t.strictEqual(location.info().cacheoccupants, 2);
        t.strictEqual(location2.info().cacheoccupants, 2);
        t.strictEqual(location2.info().copybythisprocess, copybythisprocess);
        location2.close();
        t.strictEqual(location.info().cacheoccupants, 1);
        if (sharedname)
          location.deleteShared();
    }
    location.close();
    t.strictEqual(location.opened, false);
    t.strictEqual(location.mode, "closed");
    t.strictEqual(location.ipv6, false);
  });
  t.end();
});

test("should query IPv4 and IPv4 -> IPv6 embedded addresses", function(t) {
  var location = new Location(IP4DBNAME);
  var results = [
    '8.8.8.8',
    '::FFFF:8.8.8.8',
    '0000:0000:0000:0000:0000:FFFF:8.8.8.8',
    '0:0:0:0:0:ffff:808:808',
    '::ffff:808:808'
  ].map(function(ip) {
    return location.query(ip);
  });

  t.plan(1 + results.length);
  t.deepEqual(results[0], googleResult);
  results.forEach(function(result) {
    t.deepEqual(results[0], result);
  });

  location.close();
});

var maskNames = [
  'COUNTRY_SHORT',
  'COUNTRY_LONG',
  'REGION',
  'CITY',
  'ISP',
  'LATITUDE',
  'LONGITUDE',
  'DOMAIN',
  'ZIPCODE',
  'TIMEZONE',
  'NETSPEED',
  'IDDCODE',
  'AREACODE',
  'WEATHERSTATIONCODE',
  'WEATHERSTATIONNAME',
  'MCC',
  'MNC',
  'MOBILEBRAND',
  'ELEVATION',
  'USAGETYPE'];

test("should query only selected attributes", function(t) {
  var location = new Location(IP4DBNAME);

  t.plan(1 + maskNames.length * 2 + 1);

  t.deepEqual(location.query('8.8.8.8', 0), {});

  maskNames.forEach(function(name) {
    var attrName = name.toLowerCase(),
        result = {};
    result[attrName] = googleResult[attrName];
    t.deepEqual(location.query('8.8.8.8', Location[name]), result);
  });

  var mask = 0, result = {};
  maskNames.forEach(function(name) {
    var attrName = name.toLowerCase();
    mask |= Location[name];
    result[attrName] = googleResult[attrName];
    t.deepEqual(location.query('8.8.8.8', mask), result);
  });

  t.strictEqual(mask, Location.ALL);

  location.close();
});

test("should query IPv6 addresses", function(t) {
  var location = new Location(IP6DBNAME);
  var results = [];

  t.plan(16 * 2 + (16*(16 - 1) / 2));

  for(var i = 0; i < 16; ++i) {
    var ip = '2A04:' + (i).toString(16) + '000::0000'
    var res = location.query(ip);
    t.ok(res)
    t.type(res, Object);
    results.forEach(function(other) {
      t.notDeepEqual(res, other, ip);
    });
    results.push(res);
  }

  location.close();
});

test("should fail querying not an IP address", function(t) {
  var location = new Location(IP6DBNAME);

  t.throws(function() { location.query() }, new Error("IP address is required"));
  t.strictEqual(location.query(42), null);
  t.strictEqual(location.query(''), null);
  t.strictEqual(location.query('8.8.8.8.8'), null);
  t.strictEqual(location.query('::8.8.8.8.8'), null);
  t.strictEqual(location.query('foo bar'), null);
  t.strictEqual(location.query('FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF'), null);
  t.end();

  location.close();
});

test("should fail querying an IPv6 address on IPv4 database", function(t) {
  var location = new Location(IP4DBNAME);

  t.strictEqual(location.query('::'), null);
  t.strictEqual(location.query('2A04::0000'), null);
  t.end();

  location.close();
});

test("should query extreme IP addresses", function(t) {
  var location = new Location(IP4DBNAME);
  var res;

  t.ok(res = location.query('0.0.0.0'));
  t.type(res, Object);
  t.ok(res = location.query('255.255.255.255'));
  t.type(res, Object);

  location.close();

  location = new Location(IP6DBNAME);

  t.ok(res = location.query('0.0.0.0'));
  t.type(res, Object);
  t.ok(res = location.query('255.255.255.255'));
  t.type(res, Object);
  t.ok(res = location.query('::'));
  t.type(res, Object);
  t.ok(res = location.query('FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFE'));
  t.type(res, Object);
  t.end();

  location.close();
});

test("should not open different databases into named shared memory", function(t) {
  var share = "/___IP2LocationTestLoad.shared";
  var location = new Location(IP4DBNAME, share);
  t.strictEqual(location.mode, "shared");
  var info = location.info();
  t.strictEqual(info.filename, IP4DBNAME);
  t.strictEqual(info.sharedname, share);
  t.strictEqual(info.copybythisprocess, true);
  t.equal(info.cacheoccupants, 1);

  var location2 = new Location(IP4DBNAME, share);
  info = location.info();
  t.strictEqual(info.filename, IP4DBNAME);
  t.strictEqual(info.sharedname, share);
  t.strictEqual(info.copybythisprocess, true);
  t.equal(info.cacheoccupants, 2);
  location2.close();

  t.throws(function() { new Location(IP6DBNAME, share) }, new Error("could not open IP2LOCATION database"));

  location.deleteShared();
  location.close()

  t.end();
});
