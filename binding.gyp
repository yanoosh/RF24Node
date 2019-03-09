{
  "targets": [
    {
      "target_name": "nrf24Node",
      "sources": [ 
        "nrf24Node.cc",
      ],
      "include_dirs": [
        "<!(node -e \"require('nan')\")",
        "/usr/local/include/RF24",
        "/usr/local/include/RF24Network"
      ],
      "link_settings": {
        "libraries": [
          '-lrf24 -lrf24network'
        ]
      }
    }
  ]
}
