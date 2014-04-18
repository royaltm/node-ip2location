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

File IO example (the slowest, but memory saving):

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

Memory cache example (fast but loads the whole database into memory):

    location = new Ip2Location('path/to/ip2location_database.bin', 'cache');
    location.mode == "cache";
    location.close();

Shared memory example (like "cache" but shares memory between instances and processes):

    location = new Ip2Location('path/to/ip2location_database.bin', 'shared');
    location.mode == "shared";
    location.close();

The default name used for the shared memory is "/IP2location_Shm"
However you are free to pick another name (the name must begin with a slash "/" character):

    location = new Ip2Location('path/to/ip2location_database.bin', '/MyDearMemory');
    location.mode == "shared";
    location.info().shared == "/MyDearMemory";

Each consecutive call to `new Ip2Location(dbname, "shared")` will try to re-use existing shared memory.

On POSIX sytems:

A call to location.close() will not delete the shared memory, it will only detach process from it. To delete the shared memory call:

    location.deleteShared();

Before `close()`.
When the above function is called, and if any other process is attached to the shared memory, it will only delete the name of the shared memory. The other processes will continue to use the shared memory and it will be freed only after last attached process is detached from it.

Please refer to shm_open and shm_unlink man pages for more info.
