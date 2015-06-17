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
