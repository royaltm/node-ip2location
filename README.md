#Node.js addon module for IP2Location

Based on the IP2Location C library from official site

http://www.ip2location.com/developers/c

## Installation

    npm install wisdio/node-ip2location

## Package

    package.json:

    "dependencies": {
      "node-ip2location": "wisdio/node-ip2location"
    }

## Example

File IO example:

    Ip2Location = require('node-ip2location');
    location = new Ip2Location('path/to/ip2location_database.bin');

    location.query('8.8.8.8');
    { country_short: ....,
      country_long: ....,
      region: ....,
      city: ....,
      latitude: ....,
      longitude: ....,
      elevation: .... }

    location.query('8.8.8.8', Ip2Location.COUNTRYSHORT | Ip2Location.LATITUDE | Ip2Location.LONGITUDE);
    { country_short: ....,
      latitude: ....,
      longitude: ....}

    location.opened == true
    location.mode   == "file"
    location.close();
    location.opened == false
    location.mode   == "closed"

Memory example:

    location = new Ip2Location('path/to/ip2location_database.bin', 'cache');
    location.mode == "cache";
    location.close();

Shared memory example:

    location = new Ip2Location('path/to/ip2location_database.bin', 'shared');
    location.mode == "shared";
    location.close();

Each call to new Ip2Location(dbname, "shared") will try to re-open shared memory first.
To unlink shared memory (for e.g. database upgrade) call:

    Ip2Location.deleteShared();
