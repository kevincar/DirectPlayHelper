#ifndef TESTS_INCLUDE_DPPL_DPMSGTMPS_H_
#define TESTS_INCLUDE_DPPL_DPMSGTMPS_H_

#define TMP_ENUMSESSIONS                                                    \
  {                                                                         \
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc4, 0xb3, 0x01, 0xa2, 0x20, 0x3e, \
        0x08, 0x00, 0x45, 0x00, 0x00, 0x50, 0x61, 0xdb, 0x00, 0x00, 0x40,   \
        0x11, 0x56, 0xd3, 0xc0, 0xa8, 0x01, 0x47, 0xff, 0xff, 0xff, 0xff,   \
        0xd1, 0x20, 0xba, 0x08, 0x00, 0x3c, 0x11, 0x92, 0x34, 0x00, 0xb0,   \
        0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61, 0x79, 0x02,   \
        0x00, 0x0e, 0x00, 0xc0, 0x13, 0x06, 0xbf, 0x79, 0xde, 0xd0, 0x11,   \
        0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b, 0x00, 0x00, 0x00,   \
        0x00, 0x52, 0x00, 0x00, 0x00                                        \
  }

#define TMP_ENUMSESSIONSREPLY                                               \
  {                                                                         \
    0xa6, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,   \
        0x79, 0x01, 0x00, 0x0e, 0x00, 0x50, 0x00, 0x00, 0x00, 0xc0, 0x00,   \
        0x00, 0x00, 0x4a, 0xc1, 0xcd, 0x87, 0xf0, 0x15, 0x21, 0x47, 0x8f,   \
        0x94, 0x76, 0xc8, 0x4c, 0xef, 0x3c, 0xbb, 0xc0, 0x13, 0x06, 0xbf,   \
        0x79, 0xde, 0xd0, 0x11, 0x99, 0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad,   \
        0x4b, 0x04, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x95, 0x01, 0x00,   \
        0x00, 0x00, 0x00, 0xa4, 0x00, 0x52, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x0a, 0x00, 0x08, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x5c, 0x00, 0x00,   \
        0x00, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00, 0x6e, 0x00,   \
        0x27, 0x00, 0x73, 0x00, 0x20, 0x00, 0x47, 0x00, 0x61, 0x00, 0x6d,   \
        0x00, 0x65, 0x00, 0x3a, 0x00, 0x4a, 0x00, 0x4b, 0x00, 0x31, 0x00,   \
        0x4d, 0x00, 0x50, 0x00, 0x3a, 0x00, 0x6d, 0x00, 0x31, 0x00, 0x30,   \
        0x00, 0x2e, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00, 0x00, 0x00    \
  }

#define TMP_REQUESTPLAYERID_1                                               \
  {                                                                         \
    0x20, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,   \
        0x79, 0x05, 0x00, 0x0e, 0x00, 0x09, 0x00, 0x00, 0x00                \
  }

#define TMP_REQUESTPLAYERID_2                                               \
  {                                                                         \
    0x78, 0x4f, 0x43, 0x62, 0xf2, 0x59, 0xc4, 0xb3, 0x01, 0xa2, 0x20, 0x3e, \
        0x08, 0x00, 0x45, 0x00, 0x00, 0x54, 0x00, 0x00, 0x40, 0x00, 0x40,   \
        0x06, 0xb6, 0xcc, 0xc0, 0xa8, 0x01, 0x47, 0xc0, 0xa8, 0x01, 0x40,   \
        0xec, 0xfe, 0x08, 0xfc, 0x64, 0x5f, 0xcb, 0x75, 0x8a, 0x4c, 0x6f,   \
        0x25, 0x80, 0x18, 0x08, 0x0a, 0x54, 0x0c, 0x00, 0x00, 0x01, 0x01,   \
        0x08, 0x0a, 0x21, 0x2d, 0xd3, 0x16, 0x0b, 0x60, 0xae, 0xe4, 0x20,   \
        0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,   \
        0x79, 0x05, 0x00, 0x0e, 0x00, 0x08, 0x00, 0x00, 0x00                \
  }

#define TMP_REQUESTPLAYERREPLY_1                                               \
  {                                                                            \
    0x44, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00,    \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,      \
        0x79, 0x07, 0x00, 0x0e, 0x00, 0xad, 0xfd, 0x97, 0x01, 0x00, 0x00,      \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 \
  }

#define TMP_REQUESTPLAYERREPLY_2                                            \
  {                                                                         \
    0xc4, 0xb3, 0x01, 0xa2, 0x20, 0x3e, 0x78, 0x4f, 0x43, 0x62, 0xf2, 0x59, \
        0x08, 0x00, 0x45, 0x00, 0x00, 0x78, 0x00, 0x00, 0x40, 0x00, 0x40,   \
        0x06, 0xb6, 0xa8, 0xc0, 0xa8, 0x01, 0x40, 0xc0, 0xa8, 0x01, 0x47,   \
        0xc3, 0xee, 0x08, 0xfc, 0x95, 0x55, 0xc7, 0x2e, 0x6d, 0xcc, 0x9c,   \
        0x94, 0x80, 0x18, 0x08, 0x0a, 0xdf, 0xcd, 0x00, 0x00, 0x01, 0x01,   \
        0x08, 0x0a, 0x0b, 0x60, 0xaf, 0xac, 0x21, 0x2d, 0xd2, 0xdb, 0x44,   \
        0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,   \
        0x79, 0x07, 0x00, 0x0e, 0x00, 0xac, 0xfd, 0x94, 0x01, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00,                                                               \
  }

#define TMP_ADDFORWARDREQUEST                                                  \
  {                                                                            \
    0x86, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00,    \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,      \
        0x79, 0x13, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xad, 0xfd,      \
        0x97, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x6c,      \
        0x00, 0x00, 0x00, 0x50, 0x00, 0x00, 0x00, 0x09, 0x00, 0x00, 0x00,      \
        0xad, 0xfd, 0x97, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
        0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
        0x00, 0x00, 0xad, 0xfd, 0x97, 0x01, 0x30, 0x00, 0x00, 0x00, 0x0e,      \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x08, 0xfc,      \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
        0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,      \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x95, 0x01 \
  }

#define TMP_SUPERENUMPLAYERSREPLY                                           \
  {                                                                         \
    0x69, 0x01, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,   \
        0x79, 0x29, 0x00, 0x0e, 0x00, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0xaa, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x24,   \
        0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x50, 0x00, 0x00, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x4a, 0xc1, 0xcd,   \
        0x87, 0xf0, 0x15, 0x21, 0x47, 0x8f, 0x94, 0x76, 0xc8, 0x4c, 0xef,   \
        0x3c, 0xbb, 0xc0, 0x13, 0x06, 0xbf, 0x79, 0xde, 0xd0, 0x11, 0x99,   \
        0xc9, 0x00, 0xa0, 0x24, 0x76, 0xad, 0x4b, 0x04, 0x00, 0x00, 0x00,   \
        0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0xa9, 0xfd, 0x95, 0x01, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x00,   \
        0x52, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0a, 0x00, 0x08, 0x00, 0xb4,   \
        0x00, 0x00, 0x00, 0x4b, 0x00, 0x65, 0x00, 0x76, 0x00, 0x69, 0x00,   \
        0x6e, 0x00, 0x27, 0x00, 0x73, 0x00, 0x20, 0x00, 0x47, 0x00, 0x61,   \
        0x00, 0x6d, 0x00, 0x65, 0x00, 0x3a, 0x00, 0x4a, 0x00, 0x4b, 0x00,   \
        0x31, 0x00, 0x4d, 0x00, 0x50, 0x00, 0x3a, 0x00, 0x6d, 0x00, 0x31,   \
        0x00, 0x30, 0x00, 0x2e, 0x00, 0x6a, 0x00, 0x6b, 0x00, 0x6c, 0x00,   \
        0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0xad,   \
        0xfd, 0x97, 0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00,   \
        0x20, 0x02, 0x00, 0x08, 0xfc, 0xc0, 0xa8, 0x01, 0x47, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0xc0,   \
        0xa8, 0x01, 0x47, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x10, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xa9, 0xfd, 0x94,   \
        0x01, 0x04, 0x00, 0x00, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x20, 0x02,   \
        0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,   \
        0x00, 0x00, 0x0c, 0x00, 0x00, 0x00, 0xa8, 0xfd, 0x94, 0x01, 0x05,   \
        0x00, 0x00, 0x00, 0xa9, 0xfd, 0x94, 0x01, 0x4b, 0x00, 0x65, 0x00,   \
        0x76, 0x00, 0x69, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x20, 0x02, 0x00,   \
        0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                     \
  }

#define TMP_CREATEPLAYER                                                    \
  {                                                                         \
    0x94, 0x00, 0xb0, 0xfa, 0x02, 0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x70, 0x6c, 0x61,   \
        0x79, 0x08, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0xac, 0xfd,   \
        0x94, 0x01, 0x00, 0x00, 0x00, 0x00, 0x1c, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x5e, 0x00, 0x00, 0x00, 0x08, 0x00, 0x00, 0x00,   \
        0xac, 0xfd, 0x94, 0x01, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0xad, 0xfd, 0x97, 0x01, 0x30, 0x00, 0x00, 0x00, 0x0e,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a, 0x00, 0x6f, 0x00,   \
        0x72, 0x00, 0x64, 0x00, 0x61, 0x00, 0x6e, 0x00, 0x00, 0x00, 0x02,   \
        0x00, 0x08, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x09, 0x2e, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00                                              \
  }

#define TMP_DATACOMMAND_20                                                  \
  {                                                                         \
    0xa8, 0xfd, 0x94, 0x01, 0xac, 0xfd, 0x94, 0x01, 0x20, 0x00, 0x00, 0x00, \
        0x00, 0x00, 0x00, 0x00, 0xa8, 0xfd, 0x94, 0x01, 0x10, 0x00, 0x07,   \
        0x00, 0x00, 0x00, 0xa8, 0xfd, 0x94, 0x01, 0x4b, 0x65, 0x76, 0x69,   \
        0x6e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02,   \
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,   \
        0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,   \
        0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00,   \
        0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02,   \
        0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00,   \
        0x02, 0x00, 0x00, 0x00,                                             \
  }

#define TMP_DATACOMMAND_22                                                  \
  {                                                                         \
    0xac, 0xfd, 0x94, 0x01, 0x00, 0x00, 0x00, 0x00, 0x22, 0x00, 0x00, 0x00, \
        0x6d, 0x31, 0x30, 0x2e, 0x6a, 0x6b, 0x6c, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4a,   \
        0x00, 0x6f, 0x00, 0x72, 0x00, 0x64, 0x00, 0x61, 0x00, 0x6e, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x2f, 0xfe, 0xab, 0xee    \
  }

#define TMP_DATACOMMAND_29                                                  \
  {                                                                         \
    0xa8, 0xfd, 0x94, 0x01, 0xac, 0xfd, 0x94, 0x01, 0x29, 0x00, 0x00, 0x00, \
        0x02, 0xac, 0xfd, 0x94, 0x01, 0xa8, 0xfd, 0x94, 0x01                \
  }

#endif  // TESTS_INCLUDE_DPPL_DPMSGTMPS_H_
