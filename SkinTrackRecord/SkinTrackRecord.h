 /*
  SkinTrackRecord.h - Library for SkinTrack records
*/

#ifndef SKINTRACKRECORD_H
#define SKINTRACKRECORD_H

#include "Arduino.h"

class SkinTrackRecord {
  public:
    int beacon;
    int direction;
    SkinTrackRecord();
};

#endif
