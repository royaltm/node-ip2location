var Location = require('./index')
  , location;

var MSG_NOT_SUPPORTED = "This method is not applicable for current IP2Location binary data file. Please upgrade your subscription package to install new data file.";

exports.IP2Location_init = function(binfile, mode) {
  if (location) location.close();

  if (binfile) {
    location = new Location(binfile, mode || "mmap");
  }
}

exports.IP2Location_get_all = function(myIP) {

  if (! location ) {
    return {
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
    };
  }

  return location.getAll(myIP);

}

for (var key in Location) {
  var unsupported;
  switch(key) {
    case 'ALL':
      continue;
    case 'LATITUDE':
    case 'LONGITUDE':
    case 'ELEVATION':
      unsupported = 0;
      break;
    default:
      unsupported = MSG_NOT_SUPPORTED;
  }

  (function (attr, mask, fallback) {
    exports['IP2Location_get_' + attr] = function(myIP) {
      if (location && location.opened) {
        var result = location.query(myIP, mask);
        return result ? (result[attr] || fallback) : '?';
      } else {
        return '?';
      }
    }
  })(key.toLowerCase(), Location[key], unsupported);
}
