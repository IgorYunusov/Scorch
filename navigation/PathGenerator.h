/*
 * This file is part of the TrinityCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include <windows.h>
#include <algorithm>
#include <vector>
#include <string>
#include <cmath>

#define MAX_PATH_LENGTH         256
#define MAX_POINT_PATH_LENGTH   256

#define SMOOTH_PATH_STEP_SIZE   4.0f
#define SMOOTH_PATH_SLOP        0.3f

#define VERTEX_SIZE             3
#define INVALID_POLYREF         0

#define MAX_NUMBER_OF_GRIDS     64
#define SIZE_OF_GRIDS           533.3333f
#define MAP_SIZE                (SIZE_OF_GRIDS*MAX_NUMBER_OF_GRIDS)
#define MAP_HALFSIZE            (MAP_SIZE/2)


struct Vector3
{
	Vector3() : x(0), y(0), z(0) {}
	Vector3(float_t x, float_t y, float_t z) : x(x), y(y), z(z) {}

	std::string toString() const { return std::string("Vector3 { x: " + std::to_string(x) + ", y: " + std::to_string(y) + ", z: " + std::to_string(z) + "}"); }
	inline Vector3 operator- (const Vector3& v) const { return Vector3(x - v.x, y - v.y, z - v.z); }
	inline float_t squaredLength() const { return x * x + y * y + z * z; }

	float_t x;
	float_t y;
	float_t z;
};

enum NavArea
{
	NAV_AREA_EMPTY = 0,
	// areas 1-60 will be used for destructible areas (currently skipped in vmaps, WMO with flag 1)
	// ground is the highest value to make recast choose ground over water when merging surfaces very close to each other (shallow water would be walkable)
	NAV_AREA_GROUND = 63,
	NAV_AREA_WATER = 62,
	NAV_AREA_MAGMA_SLIME = 61 // don't need to differentiate between them
};

enum NavTerrainFlag
{
	NAV_EMPTY = 0x00,
	NAV_GROUND = 1 << (63 - NAV_AREA_GROUND),
	NAV_WATER = 1 << (63 - NAV_AREA_WATER),
	NAV_MAGMA_SLIME = 1 << (63 - NAV_AREA_MAGMA_SLIME)
};

enum PathType
{
	PATHFIND_BLANK = 0x00,   // path not built yet
	PATHFIND_NORMAL = 0x01,   // normal path
	PATHFIND_SHORTCUT = 0x02,   // travel through obstacles, terrain, air, etc (old behavior)
	PATHFIND_INCOMPLETE = 0x04,   // we have partial path to follow - getting closer to target
	PATHFIND_NOPATH = 0x08,   // no valid path at all or error in generating one
	PATHFIND_NOT_USING_PATH = 0x10,   // used when we are either flying/swiming or on map w/o mmaps
	PATHFIND_SHORT = 0x20,   // path is longer or equal to its limited path length
	PATHFIND_FARFROMPOLY_START = 0x40,   // start position is far from the mmap poligon
	PATHFIND_FARFROMPOLY_END = 0x80,   // end positions is far from the mmap poligon
	PATHFIND_FARFROMPOLY = PATHFIND_FARFROMPOLY_START | PATHFIND_FARFROMPOLY_END, // start or end positions are far from the mmap poligon
};

inline bool IsValidMapCoord(float_t c)
{
	return std::isfinite(c) && (std::fabs(c) <= MAP_HALFSIZE - 0.5f);
}

inline bool IsValidMapCoord(float_t x, float_t y)
{
	return IsValidMapCoord(x) && IsValidMapCoord(y);
}

inline bool IsValidMapCoord(float_t x, float_t y, float_t z)
{
	return IsValidMapCoord(x, y) && IsValidMapCoord(z);
}

inline bool IsValidMapCoord(float_t x, float_t y, float_t z, float_t o)
{
	return IsValidMapCoord(x, y, z) && std::isfinite(o);
}

class PathGenerator
{
public:
	PathGenerator(dtNavMesh const* navMesh, dtNavMeshQuery const* navMeshQuery);
	~PathGenerator();

	// Calculate the path from owner to given destination
	// return: true if new path was calculated, false otherwise (no change needed)
	bool CalculatePath(float_t sourceX, float_t sourceY, float_t sourceZ, float_t destX, float_t destY, float_t destZ, bool forceDest = false);

	// option setters - use optional
	void SetPathLengthLimit(float_t distance) { _pointPathLimit = std::min<uint32_t>(uint32_t(distance / SMOOTH_PATH_STEP_SIZE), MAX_POINT_PATH_LENGTH); }

	// result getters
	Vector3 const& GetStartPosition() const { return _startPosition; }
	Vector3 const& GetEndPosition() const { return _endPosition; }
	Vector3 const& GetActualEndPosition() const { return _actualEndPosition; }
	std::vector<Vector3> const& GetPath() const { return _pathPoints; }
	PathType GetPathType() const { return _type; }

private:

	dtPolyRef _pathPolyRefs[MAX_PATH_LENGTH];   // array of detour polygon references
	uint32_t _polyLength;                         // number of polygons in the path

	std::vector<Vector3> _pathPoints;  // our actual (x,y,z) path to the target
	PathType _type;                     // tells what kind of path this is

	bool _forceDestination; // when set, we will always arrive at given point
	uint32_t _pointPathLimit; // limit point path size; min(this, MAX_POINT_PATH_LENGTH)

	Vector3 _startPosition;        // {x, y, z} of current location
	Vector3 _endPosition;          // {x, y, z} of the destination
	Vector3 _actualEndPosition;    // {x, y, z} of the closest possible point to given destination

	dtNavMesh const* _navMesh;              // the nav mesh
	dtNavMeshQuery const* _navMeshQuery;    // the nav mesh query used to find the path

	dtQueryFilter _filter;  // use single filter for all movements, update it when needed

	void SetStartPosition(Vector3 const& point) { _startPosition = point; }
	void SetEndPosition(Vector3 const& point) { _actualEndPosition = point; _endPosition = point; }
	void SetActualEndPosition(Vector3 const& point) { _actualEndPosition = point; }

	void Clear()
	{
		_polyLength = 0;
		_pathPoints.clear();
	}

	bool InRange(Vector3 const& p1, Vector3 const& p2, float_t r, float_t h) const;
	float_t Dist3DSqr(Vector3 const& p1, Vector3 const& p2) const;
	bool InRangeYZX(float_t const* v1, float_t const* v2, float_t r, float_t h) const;

	dtPolyRef GetPathPolyByPosition(dtPolyRef const* polyPath, uint32_t polyPathSize, float_t const* Point, float* Distance = nullptr) const;
	dtPolyRef GetPolyByLocation(float_t const* Point, float* Distance) const;
	bool HaveTile(Vector3 const& p) const;

	void BuildPolyPath(Vector3 const& startPos, Vector3 const& endPos);
	void BuildPointPath(float_t const* startPoint, float_t const* endPoint);
	void BuildShortcut();

	NavTerrainFlag GetNavTerrain(float_t x, float_t y, float_t z);
	void CreateFilter();

	// smooth path aux functions
	uint32_t FixupCorridor(dtPolyRef* path, uint32_t npath, uint32_t maxPath, dtPolyRef const* visited, uint32_t nvisited);
	bool GetSteerTarget(float_t const* startPos, float_t const* endPos, float_t minTargetDist, dtPolyRef const* path, uint32_t pathSize, float* steerPos,
		unsigned char& steerPosFlag, dtPolyRef& steerPosRef);
	dtStatus FindSmoothPath(float_t const* startPos, float_t const* endPos,
		dtPolyRef const* polyPath, uint32_t polyPathSize,
		float* smoothPath, int* smoothPathSize, uint32_t smoothPathMaxSize);

	void AddFarFromPolyFlags(bool startFarFromPoly, bool endFarFromPoly);
};