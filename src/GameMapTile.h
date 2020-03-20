#pragma once

#include "GameDefs.h"
#include "ScenarioDefs.h"

// frequently used tile rotations
// 0: -90 degrees
// 1: +90 degrees
// 2: -180 degrees
// 3: -120 degrees
// 4: +120 degress
extern const glm::mat3 g_TileRotations[5];

// subtile piece translations, quads
// 0: TOP-LEFT (NW)
// 1: TOP-RIGHT (NE)
// 2: BOTTOM-RIGHT (SE)
// 3: BOTTOM-LEFT (SW)
extern const glm::vec3 g_SubTileTranslations[4];

// gamemap block data
class GameMapTile final
{
public:
    GameMapTile();

    // reset mesh geometries for specific tile face
    // @param faceid: Face index
    void ClearTileMesh(eTileFace faceid);

    // reset mesh geometries for all tile faces
    void ClearTileMesh();

public:
    Point2D mTileLocation; // logical tile location 2D

    TerrainDefinition* mBaseTerrain = nullptr; // used to determine base terrain type, cannot be null
    TerrainDefinition* mRoomTerrain = nullptr; // overrides base terrain with room specific terrain, optional
    TerrainDefinition* mFogOfWarTerrain = nullptr; // overrides base and room terrain types when tile is hidden, optional

    ePlayerID mOwnerId = ePlayerID_Null;

    GenericRoom* mRoomInstance = nullptr; // room built on tile
    TileFaceData mFaces[eTileFace_COUNT];

    unsigned int mRandomValue = 0; // effects on visuals only
    unsigned int mFloodFillCounter = 0; // increments on each flood fill operation

    bool mIsTagged;
    bool mIsRoomInnerTile; // tile is center of 3x3 square of room, flag is valid only if tile is a part of room
    bool mIsRoomEntrance; // flag is valid only if tile is a part of room
};