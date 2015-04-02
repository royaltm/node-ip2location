var test = require("tap").test
  , dropin = require('../dropin')
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
    ip: '8.8.8.8',
    ip_no: 134744072,
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
    usagetype: 'SES',
    status: 'OK'
  };

test("should have utility functions", function(t) {
  t.type(dropin.IP2Location_get_country_short     , 'function');
  t.type(dropin.IP2Location_get_country_long      , 'function');
  t.type(dropin.IP2Location_get_region            , 'function');
  t.type(dropin.IP2Location_get_city              , 'function');
  t.type(dropin.IP2Location_get_isp               , 'function');
  t.type(dropin.IP2Location_get_latitude          , 'function');
  t.type(dropin.IP2Location_get_longitude         , 'function');
  t.type(dropin.IP2Location_get_domain            , 'function');
  t.type(dropin.IP2Location_get_zipcode           , 'function');
  t.type(dropin.IP2Location_get_timezone          , 'function');
  t.type(dropin.IP2Location_get_netspeed          , 'function');
  t.type(dropin.IP2Location_get_iddcode           , 'function');
  t.type(dropin.IP2Location_get_areacode          , 'function');
  t.type(dropin.IP2Location_get_weatherstationcode, 'function');
  t.type(dropin.IP2Location_get_weatherstationname, 'function');
  t.type(dropin.IP2Location_get_mcc               , 'function');
  t.type(dropin.IP2Location_get_mnc               , 'function');
  t.type(dropin.IP2Location_get_mobilebrand       , 'function');
  t.type(dropin.IP2Location_get_elevation         , 'function');
  t.type(dropin.IP2Location_get_usagetype         , 'function');
  t.type(dropin.IP2Location_get_all               , 'function');
  t.type(dropin.IP2Location_init                  , 'function');
  t.end();
});

test("should return uninitialized error", function(t) {
  t.deepEqual(dropin.IP2Location_get_all('0.0.0.0'), {
    "ip": "?",
    "ip_no": "?",
    "country_short": "?",
    "country_long": "?",
    "region": "?",
    "city": "?",
    "isp": "?",
    "latitude": "?",
    "longitude": "?",
    "domain": "?",
    "zipcode": "?",
    "timezone": "?",
    "netspeed": "?",
    "iddcode": "?",
    "areacode": "?",
    "weatherstationcode": "?",
    "weatherstationname": "?",
    "mcc": "?",
    "mnc": "?",
    "mobilebrand": "?",
    "elevation": "?",
    "usagetype": "?",
    "status": "RUN_INIT_FIRST"
  });
  t.end();
});

test("should open and close ip2location databases", function(t) {
  dropin.IP2Location_init(IP4DBNAME);
  t.strictEqual(dropin.IP2Location_get_all('0.0.0.0').status, "OK");
  t.strictEqual(dropin.IP2Location_get_all('::').status, "IPV6_NOT_SUPPORTED");
  dropin.IP2Location_init(IP6DBNAME);
  t.strictEqual(dropin.IP2Location_get_all('::').status, "OK");
  dropin.IP2Location_init();
  t.strictEqual(dropin.IP2Location_get_all('::').status, "MISSING_FILE");
  t.end();
});

test("should query IPv4 and IPv4 -> IPv6 embedded addresses", function(t) {
  dropin.IP2Location_init(IP4DBNAME);
  var results = [
    '8.8.8.8',
    '::FFFF:8.8.8.8',
    '0000:0000:0000:0000:0000:FFFF:8.8.8.8',
    '0:0:0:0:0:ffff:808:808',
    '::ffff:808:808',
    new Buffer([8,8,8,8]),
    new Buffer('00000000000000000000FFFF08080808', 'hex')
  ].map(function(ip) {
    return dropin.IP2Location_get_all(ip);
  });

  t.plan(1 + results.length);
  t.deepEqual(results[0], googleResult);
  results.forEach(function(result) {
    t.deepEqual(results[0], result);
  });

  dropin.IP2Location_init();
});

test("should return same records with binary and string IP", function(t) {
  dropin.IP2Location_init(IP4DBNAME);
  [
    ['1.2.3.4', new Buffer([1,2,3,4])],
    ['8.8.4.4', new Buffer([8,8,4,4])],
    ['::FFFF:8.8.4.4', new Buffer('00000000000000000000FFFF08080404', 'hex')],
    ['10.0.0.1', new Buffer([10,0,0,1])]
  ].forEach(function(addresses) {
    results = addresses.map(function(addr) { return dropin.IP2Location_get_all(addr) });
    t.type(results[0], Object);
    t.ok(results[0]);
    t.deepEqual(results[0], results[1], addresses[0]);
  });

  dropin.IP2Location_init(IP6DBNAME);
  [
    ['2A04::0000', new Buffer('2A040000000000000000000000000000', 'hex')],
    ['2A04:ae3d:0000:0000::0000', new Buffer('2A04ae3d000000000000000000000000', 'hex')]
  ].forEach(function(addresses) {
    results = addresses.map(function(addr) { return dropin.IP2Location_get_all(addr) });
    t.type(results[0], Object);
    t.ok(results[0]);
    t.notDeepEqual(results[0].ip, results[1].ip, addresses[0]);
    t.strictEqual(results[0].ip, addresses[0]);
    t.deepEqual(results[1].ip, addresses[1]);
    results[1].ip = results[0].ip;
    t.deepEqual(results[0], results[1], addresses[0]);
  });

  dropin.IP2Location_init();
  t.end();
});

var properties = [
  'country_short',
  'country_long',
  'region',
  'city',
  'isp',
  'latitude',
  'longitude',
  'domain',
  'zipcode',
  'timezone',
  'netspeed',
  'iddcode',
  'areacode',
  'weatherstationcode',
  'weatherstationname',
  'mcc',
  'mnc',
  'mobilebrand',
  'elevation',
  'usagetype'];

test("should query only selected attributes", function(t) {
  dropin.IP2Location_init(IP4DBNAME);

  t.plan(properties.length);

  properties.forEach(function(name) {
    t.strictEqual(dropin['IP2Location_get_' + name]('8.8.8.8'), googleResult[name]);
  });

  dropin.IP2Location_init();
});

test("should query IPv6 addresses", function(t) {
  dropin.IP2Location_init(IP6DBNAME);
  var results = [];

  t.plan(16 * 2 + (16*(16 - 1) / 2));

  for(var i = 0; i < 16; ++i) {
    var ip = '2A04:' + (i).toString(16) + '000::0000'
    var res = dropin.IP2Location_get_all(ip);
    t.ok(res)
    t.type(res, Object);
    results.forEach(function(other) {
      t.notDeepEqual(res, other, ip);
    });
    results.push(res);
  }

  dropin.IP2Location_init();
});

test("should fail querying not an IP address", function(t) {
  dropin.IP2Location_init(IP6DBNAME);

  t.strictEqual(dropin.IP2Location_get_all().status, "INVALID_IP_ADDRESS");
  t.strictEqual(dropin.IP2Location_get_all(42).status, "INVALID_IP_ADDRESS");
  t.strictEqual(dropin.IP2Location_get_all('').status, "INVALID_IP_ADDRESS");
  t.strictEqual(dropin.IP2Location_get_all('8.8.8.8.8').status, "INVALID_IP_ADDRESS");
  t.strictEqual(dropin.IP2Location_get_all('::8.8.8.8.8').status, "INVALID_IP_ADDRESS");
  t.strictEqual(dropin.IP2Location_get_all('foo bar').status, "INVALID_IP_ADDRESS");
  t.strictEqual(dropin.IP2Location_get_all(
    'FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF').status, "IP_ADDRESS_NOT_FOUND");
  t.end();

  dropin.IP2Location_init();
});

test("should fail querying an IPv6 address on IPv4 database", function(t) {
  dropin.IP2Location_init(IP4DBNAME);

  t.strictEqual(dropin.IP2Location_get_all('::').status, "IPV6_NOT_SUPPORTED");
  t.strictEqual(dropin.IP2Location_get_all('2A04::0000').status, "IPV6_NOT_SUPPORTED");
  t.end();

  dropin.IP2Location_init();
});

test("should query extreme IP addresses", function(t) {
  dropin.IP2Location_init(IP4DBNAME);
  var res;

  t.ok(res = dropin.IP2Location_get_all('0.0.0.0'));
  t.type(res, Object);
  t.ok(res = dropin.IP2Location_get_all('255.255.255.255'));
  t.type(res, Object);

  dropin.IP2Location_init(IP6DBNAME);

  t.ok(res = dropin.IP2Location_get_all('0.0.0.0'));
  t.type(res, Object);
  t.ok(res = dropin.IP2Location_get_all('255.255.255.255'));
  t.type(res, Object);
  t.ok(res = dropin.IP2Location_get_all('::'));
  t.type(res, Object);
  t.ok(res = dropin.IP2Location_get_all('FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFF:FFFE'));
  t.type(res, Object);
  t.end();

  dropin.IP2Location_init();
});
