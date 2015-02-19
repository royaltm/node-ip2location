Native IP2Location library for node.js
======================================

This library is a complete re-write of IP2LOCATION database client.
Built for speed.

[![Build Status][BS img]][Build Status]

Data structures learnt from the IP2Location C library:

http://www.ip2location.com/developers/c-7

Sample binary databases are available [here][ip2Location-devel-db].

## Installation

    npm install ip2location-native

## Usage

```js
    Location = require('ip2location-native')
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
The default mode is "file" - without caching. It conserves memory but reading
is done with blocking IO.

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

Every other process calling `new Location(dbname, "shared")` will try to re-open
existing shared memory.

The default name used for the shared memory is "/IP2location_Shm".
However you are free to pick another (the name must begin with a slash - "/")

```js
    location = new Location('path/to/ip2location_database.bin', '/MyDatabase1')
    location.mode == "shared"
    location.info().sharedname == "/MyDatabase1"
```

####On POSIX sytems:

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

####On Windows:

Last process calling `location.close()` will delete the shared memory.

## Performance

On tested system the library in default IO mode with IP2LOCATION-LITE-5
database spends on average 25µs per ip lookup returning all available record
entities. With caching enabled it speeds up to 5µs / lookup (~200 000 / s).

This is about at least 200 times faster then the pure js [IP2Location module][ip2location-nodejs-github].
Further speed up is available by limiting the set of fields retrieved from
the database with the second argument to `query()`.

```
node test/bench IP2LOCATION-DATABASE.BIN access_mode iterations mask
```

## Resilient database access

The library takes additional precaution when dealing with database files.
The format verification routine prevents accessing wrong format or corrupted
files.

## Drop-in replacement

The drop-in module allows you to replace the official [IP2Location library][ip2location-nodejs]
without touching your code, except for:

```js
    var ip2location = require('ip2location-native/dropin')

    ip2location.IP2Location_init('path/to/ip2location_database.bin')
    ip2location.IP2Location_get_all('8.8.8.8')
```

##Database dictionary

Do you want to know all available unique location property values?
Sorted, grouped by property type and structured? No problem.
With this module it's possible to dump database dictionaries using
`createDictionary([FIELD_MASK])` method.

```js
  var dict = location.createDictionary(Location.COUNTRY_LONG|Location.REGION|Location.CITY)
  dict._index.indexOf('PL') == 174
  dict.PL.country_long == 'Poland'
  dict.PL.region._index.indexOf('Mazowieckie') == 6
  dict.PL.region.Mazowieckie.indexOf('Warsaw') == 251
```

## Notes

This module was tested on Linux (x64) and MS Windows (x64 and x86) with
node 0.8, 0.10, 0.11, 0.12 and io.js.

## Licence

LGPL

[Build Status]: https://travis-ci.org/advertine/node-ip2location
[BS img]: https://travis-ci.org/advertine/node-ip2location.svg
[ip2location-nodejs]: http://www.ip2location.com/developers/nodejs
[ip2location-nodejs-github]: https://github.com/ip2location-nodejs/IP2Location
[ip2Location-devel-db]: http://www.ip2location.com/developers#sample_ip2location_databases_bin
