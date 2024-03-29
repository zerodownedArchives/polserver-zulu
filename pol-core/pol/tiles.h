/*
History
=======
2009/12/02 Turley:    added config.max_tile_id - Tomi


Notes
=======

*/

#ifndef TILES_H
#define TILES_H

#include "../clib/rawtypes.h"

class Tile
{
public:
    string desc;
    unsigned long uoflags; // USTRUCT_TILE::*
    unsigned long flags; // FLAG::*
    u8 layer;
    u8 height;
    u8 weight; // todo mult, div
};

extern Tile *tile;

#endif
