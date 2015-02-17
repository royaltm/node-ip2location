#Node.js addon module for IP2Location

This is a complete re-write of database client library, optimized for speed.

There is no official binary database documentation so this code
is based on the IP2Location C library from official site:

http://www.ip2location.com/developers/c

It's also inspired by [this](https://github.com/bolgovr/node-ip2location) library binding.

However this implementation does not need any external library.

## Installation

    npm install wisdio/node-ip2location

## Package

    package.json:

    "dependencies": {
      "node-ip2location": "wisdio/node-ip2location"
    }

## Example

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

    # retrieve only country short, latitude and longitude
    location.query('8.8.8.8', Ip2Location.COUNTRY_SHORT | Ip2Location.LATITUDE | Ip2Location.LONGITUDE);
    { country_short: ....,
      latitude: ....,
      longitude: ....}

    location.opened == true
    location.mode   == "file"
    location.close();
    location.opened == false
    location.mode   == "closed"

The second, optional constructor argument configures memory caching mode.
The default mode is without caching - file IO only.


Memory cache example (loads the whole database into local memory):

    location = new Ip2Location('path/to/ip2location_database.bin', 'cache');
    location.mode == "cache";

Memory map example (maps database file into memory)

    location = new Ip2Location('path/to/ip2location_database.bin', 'mmap');
    location.mode == "mmap";

Shared memory example (opens named shared memory):

    location = new Ip2Location('path/to/ip2location_database.bin', 'shared');
    location.mode == "shared";

Each consecutive call to `new Ip2Location(dbname, "shared")` will try to re-use existing shared memory.

The default name used for the shared memory is "/IP2location_Shm"
However you are free to pick another (the name must begin with a slash "/" character):

    location = new Ip2Location('path/to/ip2location_database.bin', '/MyDearMemory');
    location.mode == "shared";
    location.info().sharedname == "/MyDearMemory";

On POSIX sytems:

A call to location.close() will not delete the shared memory, it will only detach process from it. To delete the shared memory call:

    location.deleteShared();

Before `close()`.
When the above function is called, and if any other process is attached to the shared memory, it will only delete the name of the shared memory. The other processes will continue to use the shared memory and it will be freed only after last attached process is detached from it.

Please refer to shm_open and shm_unlink man pages for more info.

## Performance

On tested system the library in default IO mode with IP2LOCATION-LITE-5
database spends on average 25µs per ip lookup returning all available record
entities. With caching enabled it speeds up to 5µs / lookup (~200 000 / s).

This is about at least 200 times faster then the legacy [ip2location-nodejs](https://github.com/ip2location-nodejs/IP2Location) implementation in pure js.
Further speed up is available by limiting the set of fields retrieved from
the database, with the second argument to `query()`.

## Notes

This addon is highly experimental, might not compile on many systems, so use at your own risk.

## Licence

LGPL

