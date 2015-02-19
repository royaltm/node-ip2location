Node.js native module for IP2Location
=====================================

This is a complete re-write of IP2LOCATION database client, built for speed.

[![Build Status][BS img]][Build Status]

Data structures learnt from the IP2Location C library from the official site:

http://www.ip2location.com/developers/c-7

## Installation

    npm install advertine/node-ip2location

## Package

    package.json:

    "dependencies": {
      "node-ip2location": "advertine/node-ip2location"
    }

## Usage

```js
    Location = require('node-ip2location')
    location = new Location('path/to/ip2location_database.bin', 'mmap')

    location.query('8.8.8.8')
    { country_short: ....,
      country_long: ....,
      region: ....,
      city: ....,
      latitude: ....,
      longitude: ....,
      elevation: .... }

    # retrieve only country short, latitude and longitude
    location.query('8.8.8.8', Location.COUNTRY_SHORT | Location.LATITUDE | Location.LONGITUDE)
    { country_short: ....,
      latitude: ....,
      longitude: .... }

    location.close()
```

## Caching

The second, optional constructor argument configures database access mode.
The default mode is "file" - without caching. It is memory conserving but
reads database with blocking IO only.

* "cache" loads the whole database into a local memory:

```js
    location = new Location('path/to/ip2location_database.bin', 'cache')
    location.mode == "cache"
```

* "mmap" maps database file into memory shared between processes:

```js
    location = new Location('path/to/ip2location_database.bin', 'mmap')
    location.mode == "mmap"
```

* "shared" loads whole database into named shared memory:

```js
    location = new Location('path/to/ip2location_database.bin', 'shared')
    location.mode == "shared"
```

Each consecutive call to `new Location(dbname, "shared")` will try to re-open
existing shared memory.

The default name used for the shared memory is "/IP2location_Shm".
However you are free to pick another (the name must begin with a slash - "/")

```js
    location = new Location('path/to/ip2location_database.bin', '/MyDatabase1')
    location.mode == "shared"
    location.info().sharedname == "/MyDatabase1"
```

On POSIX sytems:

A call to `location.close()` will not delete the shared memory, it will only
detach process from it. To delete the shared memory call:

```js
    location.deleteShared()
```

Before `close()`.
When `deleteShared` is called, and if any other process is attached to the
shared memory, the function will only delete the inode of the shared memory.
The other processes will continue to use the shared memory and it will be freed
only after the last process is detached from it (closes database).

Please refer to `shm_open` and `shm_unlink` man pages for more info.

## Performance

On tested system the library in default IO mode with IP2LOCATION-LITE-5
database spends on average 25µs per ip lookup returning all available record
entities. With caching enabled it speeds up to 5µs / lookup (~200 000 / s).

This is about at least 200 times faster then the legacy [ip2location-nodejs](https://github.com/ip2location-nodejs/IP2Location) implementation in js.
Further speed up is available by limiting the set of fields retrieved from
the database, with the second argument to `query()`.

```
node test/bench IP2LOCATION-DATABASE.BIN access_mode iterations mask
```

## Drop-in replacement

The drop-in module allows you to replace official IP2Location library, without
touching your code, except for:

```js
    var ip2location = require('node-ip2location/dropin')
    ip2location.IP2Location_init('path/to/ip2location_database.bin')
    ip2location.IP2Location_get_all('8.8.8.8')
```

line.

## Notes

This module was tested on Linux (x64) and MS Windows (x64 and x86) with
node 0.8, 0.10, 0.12 and io.js.

## Licence

LGPL

[Build Status]: https://travis-ci.org/advertine/node-ip2location
[BS img]: https://travis-ci.org/advertine/node-ip2location.svg
