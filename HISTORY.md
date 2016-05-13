v1.1.2

* bump nan to 2.3.3, fixes build with node v5 and v6

v1.1.1

* use nan converters instead of soon deprecated ->XValue()
* use selected v8:: and Nan:: symbols instead of namespaces

v1.1.0

* bump nan to 2.0.9, fixes build with iojs-3 and node v4

v1.0.6

* deps: bump nan to 1.8, fixes build problem with newest io.js
* node 0.8 support dropped

v1.0.5

* bugfix: shared memory file size rounded to multiple of pagesize on ux-es

v1.0.4

* feature: accept ip address as binary object (Buffer)

v1.0.3

* bugfix: database size check before re-opening shared memory
* compat: FreeBSD compatibility ensured

v1.0.2

* bugfix: custom shared name mode argument parsing
* test: test all access modes

v1.0.1

* database reads protected against corrupted files
