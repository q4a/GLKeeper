#include "pch.h"
#include "GameWorld.h"
#include "ScenarioLoader.h"
#include "Console.h"
#include "TerrainManager.h"
#include "RoomsManager.h"
#include "GenericRoom.h"
#include "GameObjectsManager.h"

GameWorld gGameWorld;

bool GameWorld::Initialize()
{
    if (!gTerrainManager.Initialize())
    {
        Deinit();

        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize terrain manager");
        return false;
    }

    if (!gGameObjectsManager.Initialize())
    {
        Deinit();

        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize gameobjects manager");
        return false;
    }

    if (!gRoomsManager.Initialize())
    {
        Deinit();
        
        gConsole.LogMessage(eLogMessage_Warning, "Cannot initialize rooms manager");
        return false;
    }

    return true;
}

void GameWorld::Deinit()
{
    gRoomsManager.Deinit();
    gGameObjectsManager.Deinit();
    gTerrainManager.Deinit();
}

bool GameWorld::LoadScenario(const std::string& scenarioName)
{
    ScenarioLoader scenarioLoader (mScenarioData);
    if (!scenarioLoader.LoadScenarioData(scenarioName))
    {
        gConsole.LogMessage(eLogMessage_Warning, "Cannot load scenario data");
        return false;
    }
    return true;
}

void GameWorld::EnterWorld()
{
    SetupMapData(0xDEADBEEF);

    gTerrainManager.EnterWorld();

    ConstructStartupRooms();
    gRoomsManager.EnterWorld();

    gGameObjectsManager.EnterWorld();

    gTerrainManager.BuildFullTerrainMesh();
    mTerrainCursor.EnterWorld();
}

void GameWorld::ClearWorld()
{
    mTerrainCursor.ClearWorld();
    gTerrainManager.ClearWorld();
    gGameObjectsManager.ClearWorld();
    gRoomsManager.ClearWorld();
    mScenarioData.Clear();
    mMapData.Clear();
}

void GameWorld::UpdateFrame()
{
    gGameObjectsManager.UpdateFrame();
    gRoomsManager.UpdateFrame();
    gTerrainManager.UpdateTerrainMesh();
    mTerrainCursor.UpdateFrame();
}

void GameWorld::TagTerrain(const Rectangle& tilesArea)
{
    MapTilesIterator tilesIterator = mMapData.IterateTiles(tilesArea);
    for (TerrainTile* currMapTile = tilesIterator.NextTile(); currMapTile; 
        currMapTile = tilesIterator.NextTile())
    {
        TerrainDefinition* terrainDef = currMapTile->GetTerrain();
        if (!currMapTile->mIsTagged && terrainDef->mIsTaggable)
        {
            currMapTile->SetTagged(true);
        }
    }
}

void GameWorld::UnTagTerrain(const Rectangle& tilesArea)
{
    MapTilesIterator tilesIterator = mMapData.IterateTiles(tilesArea);
    for (TerrainTile* currMapTile = tilesIterator.NextTile(); currMapTile; 
        currMapTile = tilesIterator.NextTile())
    {
        TerrainDefinition* terrainDef = currMapTile->GetTerrain();
        if (currMapTile->mIsTagged && terrainDef->mIsTaggable)
        {
            currMapTile->SetTagged(false);
        }
    }
}

void GameWorld::ConstructRoom(ePlayerID ownerID, RoomDefinition* roomDefinition, const Rectangle& tilesArea)
{
    // collect all tiles available to construction, output array is sorted
    TilesList tilesArray;
    tilesArray.reserve(tilesArea.w * tilesArea.h);

    MapTilesIterator tilesIterator = mMapData.IterateTiles(tilesArea);
    for (TerrainTile* currMapTile = tilesIterator.NextTile(); currMapTile; 
        currMapTile = tilesIterator.NextTile())
    {
        if (CanPlaceRoomOnLocation(currMapTile, ownerID, roomDefinition))
        {
            tilesArray.push_back(currMapTile);
        }
    }

    if (tilesArray.empty()) // nothing to construct
        return;

    std::set<TerrainTile*> processedTiles;
    // scan for contiguous segments
    for (TerrainTile* currentTile: tilesArray)
    {
        // is processed already ?
        if (processedTiles.find(currentTile) != processedTiles.end())
            continue;

        TilesList segmentTiles;
        MapFloodFillFlags floodfillFlags; // leave default
        mMapData.FloodFill4(segmentTiles, currentTile, tilesArea, floodfillFlags);
        // put into processed set
        for (TerrainTile* processedTile: segmentTiles)
        {
            processedTiles.insert(processedTile);
        }
        
        // find rooms that contacting with current segment and merge them all
        GenericRoom* receivingRoom = nullptr;
        EnumAdjacentRooms(segmentTiles, ownerID, [this, &receivingRoom, roomDefinition](GenericRoom* inspectRoom)
        {
            if (roomDefinition == inspectRoom->mDefinition)
            {
                if (!receivingRoom)
                {
                    receivingRoom = inspectRoom;
                    return;
                }
                receivingRoom->AbsorbRoom(inspectRoom);
            }
        });

        if (!receivingRoom)
        {
            receivingRoom = gRoomsManager.CreateRoomInstance(roomDefinition, ownerID);
        }

        // add segment tiles to room
        for (TerrainTile* processedTile: segmentTiles)
        {
            debug_assert(processedTile->mBuiltRoom == nullptr);
            processedTile->SetTerrain(&mScenarioData.mTerrainDefs[roomDefinition->mTerrainType]);
            processedTile->mOwnerID = ownerID;
        }
        receivingRoom->EnlargeRoom(segmentTiles);
    }
}

void GameWorld::SellRooms(ePlayerID ownerID, const Rectangle& tilesArea)
{
    std::unordered_map<GenericRoom*, TilesList> processRooms;

    // collect rooms and its tiles
    MapTilesIterator tilesIterator = mMapData.IterateTiles(tilesArea);
    for (TerrainTile* currMapTile = tilesIterator.NextTile(); currMapTile; 
        currMapTile = tilesIterator.NextTile())
    {
        if (CanSellRoomOnLocation(currMapTile, ownerID))
        {
            processRooms[currMapTile->mBuiltRoom].push_back(currMapTile);
            continue;
        }
    }

    for (const auto& element: processRooms)
    {
        ReleaseRoomTiles(element.first, element.second);
    }
}

bool GameWorld::CanPlaceRoomOnLocation(TerrainTile* terrainTile, ePlayerID ownerID, RoomDefinition* roomDefinition) const
{
    debug_assert(terrainTile);

    TerrainDefinition* tileTerrain = terrainTile->GetTerrain();
    if (IsRoomTypeTerrain(tileTerrain))
        return false;

    if (roomDefinition->mPlaceableOnLand)
    {
        if (!tileTerrain->mIsSolid && tileTerrain->mIsOwnable && (terrainTile->mOwnerID == ownerID))
            return true;
    }

    if (roomDefinition->mPlaceableOnLava && tileTerrain->mIsLava)
        return true;

    if (roomDefinition->mPlaceableOnWater && tileTerrain->mIsWater)
        return true;

    return false;
}

bool GameWorld::CanSellRoomOnLocation(TerrainTile* terrainTile, ePlayerID ownerID) const
{
    debug_assert(terrainTile);

    GenericRoom* roomInstance = terrainTile->mBuiltRoom;
    if (roomInstance == nullptr || ownerID != terrainTile->mOwnerID)
        return false;

    return (roomInstance->mDefinition->mBuildable == true);
}

void GameWorld::DamageTerrainTile(TerrainTile* mapTile, ePlayerID playerIdentifier, int hitPoints)
{
    debug_assert(mapTile);
    debug_assert(hitPoints > 0);

    if (mapTile->mBuiltRoom)
    {
        ReleaseRoomTiles(mapTile->mBuiltRoom, {mapTile});
    }

    TerrainDefinition* terrain = mapTile->GetTerrain();
    if (terrain->mIsLava || terrain->mIsWater || terrain->mIsImpenetrable)
        return;

    // todo: tile hp

    TerrainTypeID newTerrainType = terrain->mBecomesTerrainTypeWhenDestroyed;
    if (terrain->mTerrainType == newTerrainType)
        return;

    TerrainDefinition* newTerrain = &mScenarioData.mTerrainDefs[newTerrainType];
    mapTile->SetTagged(false);
    mapTile->SetTerrain(newTerrain);

    std::set<GenericRoom*> rooms;
    for (eDirection direction: gStraightDirections)
    {
        TerrainTile* neighbourTile = mapTile->mNeighbours[direction];
        if (neighbourTile && neighbourTile->mBuiltRoom)
        {
            rooms.insert(neighbourTile->mBuiltRoom);
        }
    }

    for (GenericRoom* genericRoom: rooms)
    {
        genericRoom->NeighbourTileChange(mapTile);
    }

    mapTile->InvalidateTileMesh();
    mapTile->InvalidateNeighbourTilesMesh();
}

void GameWorld::RepairTerrainTile(TerrainTile* mapTile, ePlayerID playerIdentifier, int hitPoints)
{
    debug_assert(mapTile);
    debug_assert(hitPoints > 0);

    TerrainDefinition* terrain = mapTile->GetTerrain();
    if (terrain->mIsLava || terrain->mIsWater || terrain->mIsImpenetrable)
        return;

    // todo: tile hp

    if (mapTile->mBuiltRoom)
        return;

    TerrainTypeID newTerrainType = terrain->mBecomesTerrainTypeWhenMaxHealth;
    if (terrain->mTerrainType == newTerrainType)
        return;

    TerrainDefinition* newTerrain = &mScenarioData.mTerrainDefs[newTerrainType];
    mapTile->SetTerrain(newTerrain);
    mapTile->mOwnerID = playerIdentifier;

    std::set<GenericRoom*> rooms;
    for (eDirection direction: gStraightDirections)
    {
        TerrainTile* neighbourTile = mapTile->mNeighbours[direction];
        if (neighbourTile && neighbourTile->mBuiltRoom)
        {
            rooms.insert(neighbourTile->mBuiltRoom);
        }
    }

    for (GenericRoom* genericRoom: rooms)
    {
        genericRoom->NeighbourTileChange(mapTile);
    }

    mapTile->InvalidateTileMesh();
    mapTile->InvalidateNeighbourTilesMesh();
}

void GameWorld::SetupMapData(unsigned int seed)
{
    Point mapDimensions (mScenarioData.mLevelDimensionX, mScenarioData.mLevelDimensionY);
    mMapData.Setup(mapDimensions, seed);

    int currentTileIndex = 0;
    for (int tiley = 0; tiley < mScenarioData.mLevelDimensionY; ++tiley)
    for (int tilex = 0; tilex < mScenarioData.mLevelDimensionX; ++tilex)
    {
        TerrainTile* currentTile = mMapData.GetMapTile(Point(tilex, tiley));
        debug_assert(currentTile);

        TerrainTypeID tileTerrainType = mScenarioData.mMapTiles[currentTileIndex].mTerrainType;
        if (IsRoomTypeTerrain(tileTerrainType))
        {
            RoomDefinition* roomDefinition = GetRoomDefinitionByTerrain(tileTerrainType);
            // acquire terrain type under the bridge
            if (!roomDefinition->mPlaceableOnLand)
            {
                switch (mScenarioData.mMapTiles[currentTileIndex].mTerrainUnderTheBridge)
                {
                    case eBridgeTerrain_Lava:
                        currentTile->mBaseTerrain = GetLavaTerrain();
                    break;

                    case eBridgeTerrain_Water:
                        currentTile->mBaseTerrain = GetWaterTerrain();
                    break;
                }
            }
            else
            {
                // claimed path is default
                currentTile->mBaseTerrain = GetPlayerColouredPathTerrain();
            }
            // set room terrain type
            currentTile->mRoomTerrain = GetTerrainDefinition(tileTerrainType);
        }
        else
        {
            currentTile->mBaseTerrain = GetTerrainDefinition(tileTerrainType);
        }

        debug_assert(currentTile->mBaseTerrain);
        // set additional tile params
        currentTile->mOwnerID = mScenarioData.mMapTiles[currentTileIndex].mOwnerIdentifier;

        ++currentTileIndex;
    }
}

void GameWorld::ConstructStartupRooms()
{
    // create rooms
    MapTilesIterator tilesIterator = mMapData.IterateTiles(Point(), mMapData.mDimensions);
    for (TerrainTile* currMapTile = tilesIterator.NextTile(); currMapTile; 
        currMapTile = tilesIterator.NextTile())
    {
        if (currMapTile->mBuiltRoom)
        {
            // room is already constructed on this tile
            continue;
        }

        TerrainDefinition* terrainDef = currMapTile->GetTerrain();
        if (IsRoomTypeTerrain(terrainDef))
        {
            ConstructStartupRoom(currMapTile);
        }
    }
}

void GameWorld::ConstructStartupRoom(TerrainTile* initialTile)
{
    debug_assert(initialTile);

    TerrainDefinition* terrainDef = initialTile->GetTerrain();
    // query room definition
    RoomDefinition* roomDefinition = GetRoomDefinitionByTerrain(terrainDef);
    if (roomDefinition == nullptr)
    {
        debug_assert(false);
        return;
    }

    TilesList floodTiles;

    MapFloodFillFlags floodFillFlags;
    floodFillFlags.mSameBaseTerrain = false;
    floodFillFlags.mSameOwner = true;
    mMapData.FloodFill4(floodTiles, initialTile,floodFillFlags);

    // create room instance
    GenericRoom* roomInstance = gRoomsManager.CreateRoomInstance(roomDefinition, initialTile->mOwnerID, floodTiles);
    debug_assert(roomInstance);
}

void GameWorld::ReleaseRoomTiles(GenericRoom* roomInstance, const TilesList& roomTiles)
{
    debug_assert(roomInstance);
    roomInstance->ReleaseTiles(roomTiles);

    // change terrain
    for (TerrainTile* roomTile: roomTiles)
    {
        roomTile->SetTerrain(nullptr);
        if (!roomInstance->mDefinition->mPlaceableOnLand && 
            (roomInstance->mDefinition->mPlaceableOnLava || roomInstance->mDefinition->mPlaceableOnWater))
        {   
            roomTile->mOwnerID = ePlayerID_Neutral; // for bridges terrain reset owner
        }
    }

    int segmentsCounter = 0;
    EnumRoomSegments(roomInstance, [this, &segmentsCounter, roomInstance](const TilesList& segmentTiles)
    {
        if (segmentsCounter++ == 0) // first segment is current room segment
            return;

        // each new segment is a new room
        GenericRoom* newRoomInstance = gRoomsManager.CreateRoomInstance(roomInstance->mDefinition, roomInstance->mOwnerID);
        newRoomInstance->AbsorbRoom(roomInstance, segmentTiles);
    });

    if (!roomInstance->HasTiles())
    {
        gRoomsManager.DestroyRoomInstance(roomInstance);
    }
}

TerrainDefinition* GameWorld::GetLavaTerrain()
{
    debug_assert(mScenarioData.mLavaTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mLavaTerrainType);
}

TerrainDefinition* GameWorld::GetWaterTerrain()
{
    debug_assert(mScenarioData.mWaterTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mWaterTerrainType);
}

TerrainDefinition* GameWorld::GetPlayerColouredPathTerrain()
{
    debug_assert(mScenarioData.mPlayerColouredPathTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mPlayerColouredPathTerrainType);
}

TerrainDefinition* GameWorld::GetPlayerColouredWallTerrain()
{
    debug_assert(mScenarioData.mPlayerColouredWallTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mPlayerColouredWallTerrainType);
}

TerrainDefinition* GameWorld::GetFogOfWarTerrain()
{
    debug_assert(mScenarioData.mFogOfWarTerrainType != TerrainType_Null);
    return GetTerrainDefinition(mScenarioData.mFogOfWarTerrainType);
}

TerrainDefinition* GameWorld::GetTerrainDefinition(const std::string& typeName)
{
    for (TerrainDefinition& currentDefinition: mScenarioData.mTerrainDefs)
    {
        if (currentDefinition.mName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

TerrainDefinition* GameWorld::GetTerrainDefinition(TerrainTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mTerrainDefs.size());
    return &mScenarioData.mTerrainDefs[typeID];
}

GameObjectDefinition* GameWorld::GetGameObjectDefinition(const std::string& typeName)
{
    for (GameObjectDefinition& currentDefinition: mScenarioData.mGameObjectDefs)
    {
        if (currentDefinition.mObjectName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

GameObjectDefinition* GameWorld::GetGameObjectDefinition(GameObjectTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mGameObjectDefs.size());
    return &mScenarioData.mGameObjectDefs[typeID];
}

RoomDefinition* GameWorld::GetRoomDefinition(const std::string& typeName)
{
    for (RoomDefinition& currentDefinition: mScenarioData.mRoomDefs)
    {
        if (currentDefinition.mRoomName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

RoomDefinition* GameWorld::GetRoomDefinition(RoomTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mRoomDefs.size());
    return &mScenarioData.mRoomDefs[typeID];
}

CreatureDefinition* GameWorld::GetCreatureDefinition(const std::string& typeName)
{
    for (CreatureDefinition& currentDefinition: mScenarioData.mCreatureDefs)
    {
        if (currentDefinition.mCreatureName == typeName)
            return &currentDefinition;
    }
    return nullptr;
}

CreatureDefinition* GameWorld::GetCreatureDefinition(CreatureTypeID typeID)
{
    debug_assert(typeID < mScenarioData.mCreatureDefs.size());
    return &mScenarioData.mCreatureDefs[typeID];
}

RoomDefinition* GameWorld::GetRoomDefinitionByTerrain(TerrainDefinition* terrainDefinition)
{
    if (terrainDefinition == nullptr)
    {
        debug_assert(false);
        return &mScenarioData.mRoomDefs[RoomType_Null];
    }

    return GetRoomDefinitionByTerrain(terrainDefinition->mTerrainType);
}

RoomDefinition* GameWorld::GetRoomDefinitionByTerrain(TerrainTypeID typeID)
{
    return GetRoomDefinition(mScenarioData.mRoomByTerrainType[typeID]);
}

bool GameWorld::IsRoomTypeTerrain(TerrainDefinition* terrainDefinition) const
{
    if (terrainDefinition == nullptr)
    {
        debug_assert(false);
        return false;
    }

    return IsRoomTypeTerrain(terrainDefinition->mTerrainType);
}

bool GameWorld::IsRoomTypeTerrain(TerrainTypeID typeID) const
{
    return mScenarioData.mRoomByTerrainType[typeID] != RoomType_Null;
}

template<typename TEnumProc>
void GameWorld::EnumAdjacentRooms(const TilesList& tilesToScan, ePlayerID ownerID, TEnumProc enumProc)
{
    std::set<GenericRoom*> processedRooms;
    for (TerrainTile* currentTile: tilesToScan)
    {
#define SCAN_NEIGHBOUR_ROOM(neigh_direction)\
    if (TerrainTile* neighbourTile = currentTile->mNeighbours[neigh_direction])\
    {\
        if (neighbourTile->mOwnerID == ownerID && neighbourTile->mBuiltRoom)\
        {\
            if (processedRooms.find(neighbourTile->mBuiltRoom) == processedRooms.end())\
            {\
                enumProc(neighbourTile->mBuiltRoom);\
                processedRooms.insert(neighbourTile->mBuiltRoom);\
            }\
        }\
    }

        SCAN_NEIGHBOUR_ROOM(eDirection_N);
        SCAN_NEIGHBOUR_ROOM(eDirection_E);
        SCAN_NEIGHBOUR_ROOM(eDirection_S);
        SCAN_NEIGHBOUR_ROOM(eDirection_W);
    }

#undef SCAN_NEIGHBOUR_ROOM
}

template<typename TEnumProc>
void GameWorld::EnumRoomSegments(GenericRoom* roomInstance, TEnumProc enumProc)
{
    debug_assert(roomInstance);

    std::set<TerrainTile*> processedTiles;
    TilesList coveredTiles = roomInstance->mRoomTiles;// intent copy
    for (TerrainTile* targetTile: coveredTiles)
    {
        if (processedTiles.find(targetTile) != processedTiles.end()) // already processed
            continue;

        TilesList segmentTiles;
        MapFloodFillFlags floodfillFlags;
        mMapData.FloodFill4(segmentTiles, targetTile, floodfillFlags);
        // add to processed tiles
        processedTiles.insert(segmentTiles.begin(), segmentTiles.end());
        enumProc(segmentTiles);
    }
}
