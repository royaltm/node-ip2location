{
  'targets': [
    {
      'target_name': 'nodeip2location',
      'sources': [
        'src/iMath/imath.c',
        'src/ip2location/IP2Location.c',
        'src/ip2location/IP2Loc_DBInterface.c',
        'src/nodeip2location.cc'
      ],
      'include_dirs': [
        'src/ip2location',
        'src/iMath'
      ]
    }
  ]
}
