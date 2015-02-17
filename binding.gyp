{
  'targets': [
    {
      'target_name': 'nodeip2location',
      'sources': [
        'src/curl/inet_pton.c',
        'src/ip2location/ip2lmemorymaplist.c',
        'src/ip2location/ip2lipaddress.c',
        'src/ip2location/ip2ldatabase.c',
        'src/ip2location/ip2location.c',
        'src/nodeip2location.cc'
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        'src/curl',
        'src/ip2location'
      ],
      'conditions': [
        ['OS=="win"', {
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ExceptionHandling': 1,
              'AdditionalOptions': [
                '/EHsc' # ExceptionHandling=1 is not enough
              ]
            }
          },
          'libraries': [
            '-lWs2_32.lib',
          ]
        }]
      ]
    }
  ]
}
