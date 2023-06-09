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

#include "PathGenerator.h"
#include "DetourCommon.h"
#include "DetourNavMeshQuery.h"

////////////////// PathGenerator //////////////////
PathGenerator::PathGenerator(dtNavMesh const* navMesh, dtNavMeshQuery const* navMeshQuery) :
	_polyLength(0), _type(PATHFIND_BLANK),
	_forceDestination(false), _pointPathLimit(MAX_POINT_PATH_LENGTH),
	_endPosition(Vector3()), _navMesh(navMesh), _navMeshQuery(navMeshQuery)
{
	memset(_pathPolyRefs, 0, sizeof(_pathPolyRefs));
	CreateFilter();
}

PathGenerator::~PathGenerator()
{
	printf("maps.mmaps ++ PathGenerator::~PathGenerator()\n");
}

void printDtStatus(dtStatus status)
{
	printf("DtStatus: Failed: %i OOM: %i, B2S: %i, OON: %i, PARTIAL: %i INVALID: %i, OCCUPIED: %i\n",
		dtStatusFailed(status), (status & DT_OUT_OF_MEMORY) != 0, 
		(status & DT_BUFFER_TOO_SMALL) != 0, (status & DT_OUT_OF_NODES) != 0, 
		(status & DT_PARTIAL_RESULT) != 0, (status & DT_INVALID_PARAM) != 0,
		(status & DT_ALREADY_OCCUPIED) != 0);
}

bool PathGenerator::CalculatePath(float_t sourceX, float_t sourceY, float_t sourceZ, float_t destX, float_t destY, float_t destZ, bool forceDest)
{
	if (!IsValidMapCoord(destX, destY, destZ) || !IsValidMapCoord(sourceX, sourceY, sourceZ))
		return false;

	Vector3 dest(destX, destY, destZ);
	SetEndPosition(dest);

	Vector3 start(sourceX, sourceY, sourceZ);
	SetStartPosition(start);

	_forceDestination = forceDest;

	// make sure navMesh works - we can run on map w/o mmap
	// check if the start and end point have a .mmtile loaded (can we pass via not loaded tile on the way?)
	if (!_navMesh || !_navMeshQuery || !HaveTile(start) || !HaveTile(dest))
	{
		printf("Don't have a requirement\n");
		BuildShortcut();
		_type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
		return false;
	}

	BuildPolyPath(start, dest);
	return true;
}

dtPolyRef PathGenerator::GetPathPolyByPosition(dtPolyRef const* polyPath, uint32_t polyPathSize, float_t const* point, float_t* distance) const
{
	if (!polyPath || !polyPathSize)
		return INVALID_POLYREF;

	dtPolyRef nearestPoly = INVALID_POLYREF;
	float_t minDist = FLT_MAX;

	for (uint32_t i = 0; i < polyPathSize; ++i)
	{
		float_t closestPoint[VERTEX_SIZE];
		if (dtStatusFailed(_navMeshQuery->closestPointOnPoly(polyPath[i], point, closestPoint, nullptr)))
			continue;

		float_t d = dtVdistSqr(point, closestPoint);
		if (d < minDist)
		{
			minDist = d;
			nearestPoly = polyPath[i];
		}

		if (minDist < 1.0f) // shortcut out - close enough for us
			break;
	}

	if (distance)
		*distance = dtMathSqrtf(minDist);

	return (minDist < 3.0f) ? nearestPoly : INVALID_POLYREF;
}

dtPolyRef PathGenerator::GetPolyByLocation(float_t const* point, float_t* distance) const
{
	// first we check the current path
	// if the current path doesn't contain the current poly,
	// we need to use the expensive navMesh.findNearestPoly
	dtPolyRef polyRef = GetPathPolyByPosition(_pathPolyRefs, _polyLength, point, distance);
	if (polyRef != INVALID_POLYREF)
		return polyRef;

	// we don't have it in our old path
	// try to get it by findNearestPoly()
	// first try with low search box
	float_t extents[VERTEX_SIZE] = { 3.0f, 5.0f, 3.0f };    // bounds of poly search area
	float_t closestPoint[VERTEX_SIZE] = { 0.0f, 0.0f, 0.0f };
	if (dtStatusSucceed(_navMeshQuery->findNearestPoly(point, extents, &_filter, &polyRef, closestPoint)) && polyRef != INVALID_POLYREF)
	{
		*distance = dtVdist(closestPoint, point);
		return polyRef;
	}

	// still nothing ..
	// try with bigger search box
	// Note that the extent should not overlap more than 128 polygons in the navmesh (see dtNavMeshQuery::findNearestPoly)
	extents[1] = 50.0f;
	if (dtStatusSucceed(_navMeshQuery->findNearestPoly(point, extents, &_filter, &polyRef, closestPoint)) && polyRef != INVALID_POLYREF)
	{
		*distance = dtVdist(closestPoint, point);
		return polyRef;
	}

	*distance = FLT_MAX;
	return INVALID_POLYREF;
}

void PathGenerator::BuildPolyPath(Vector3 const& startPos, Vector3 const& endPos)
{
	// *** getting start/end poly logic ***

	float_t distToStartPoly, distToEndPoly;
	float_t startPoint[VERTEX_SIZE] = { startPos.y, startPos.z, startPos.x };
	float_t endPoint[VERTEX_SIZE] = { endPos.y, endPos.z, endPos.x };

	dtPolyRef startPoly = GetPolyByLocation(startPoint, &distToStartPoly);
	dtPolyRef endPoly = GetPolyByLocation(endPoint, &distToEndPoly);

	_type = PathType(PATHFIND_NORMAL);

	// we have a hole in our mesh
	// make shortcut path and mark it as NOPATH ( with flying and swimming exception )
	// its up to caller how he will use this info
	if (startPoly == INVALID_POLYREF || endPoly == INVALID_POLYREF)
	{
		printf("maps.mmaps ++ BuildPolyPath :: (startPoly == 0 || endPoly == 0)\n");
		BuildShortcut();
		_type = PATHFIND_NOPATH;
		return;
	}

	// we may need a better number here
	bool startFarFromPoly = distToStartPoly > 7.0f;
	bool endFarFromPoly = distToEndPoly > 7.0f;

	if (startFarFromPoly || endFarFromPoly)
	{
		printf("maps.mmaps ++ BuildPolyPath :: farFromPoly distToStartPoly=%.3f distToEndPoly=%.3f\n", distToStartPoly, distToEndPoly);
		float_t closestPoint[VERTEX_SIZE];
		// we may want to use closestPointOnPolyBoundary instead
		if (dtStatusSucceed(_navMeshQuery->closestPointOnPoly(endPoly, endPoint, closestPoint, nullptr)))
		{
			dtVcopy(endPoint, closestPoint);
			SetActualEndPosition(Vector3(endPoint[2], endPoint[0], endPoint[1]));
		}
		_type = PathType(PATHFIND_INCOMPLETE);
		AddFarFromPolyFlags(startFarFromPoly, endFarFromPoly);
	}

	// *** poly path generating logic ***

	// start and end are on same polygon
	// handle this case as if they were 2 different polygons, building a line path split in some few points
	if (startPoly == endPoly)
	{
		printf("maps.mmaps ++ BuildPolyPath :: (startPoly == endPoly)\n");

		_pathPolyRefs[0] = startPoly;
		_polyLength = 1;

		if (startFarFromPoly || endFarFromPoly)
		{
			_type = PathType(PATHFIND_INCOMPLETE);
			AddFarFromPolyFlags(startFarFromPoly, endFarFromPoly);
		}
		else
			_type = PATHFIND_NORMAL;

		BuildPointPath(startPoint, endPoint);
		return;
	}

	// look for startPoly/endPoly in current path
	/// @todo we can merge it with getPathPolyByPosition() loop
	bool startPolyFound = false;
	bool endPolyFound = false;
	uint32_t pathStartIndex = 0;
	uint32_t pathEndIndex = 0;

	if (_polyLength)
	{
		for (; pathStartIndex < _polyLength; ++pathStartIndex)
		{
			// here to catch few bugs
			if (_pathPolyRefs[pathStartIndex] == INVALID_POLYREF)
			{
				printf("maps.mmaps Invalid poly ref in BuildPolyPath. _polyLength: %u, pathStartIndex: %u, startPos: %s, endPos: %s\n",
					_polyLength, pathStartIndex, startPos.toString().c_str(), endPos.toString().c_str());

				break;
			}

			if (_pathPolyRefs[pathStartIndex] == startPoly)
			{
				startPolyFound = true;
				break;
			}
		}

		for (pathEndIndex = _polyLength - 1; pathEndIndex > pathStartIndex; --pathEndIndex)
			if (_pathPolyRefs[pathEndIndex] == endPoly)
			{
				endPolyFound = true;
				break;
			}
	}

	if (startPolyFound && endPolyFound)
	{
		printf("maps.mmaps ++ BuildPolyPath :: (startPolyFound && endPolyFound)\n");

		// we moved along the path and the target did not move out of our old poly-path
		// our path is a simple subpath case, we have all the data we need
		// just "cut" it out

		_polyLength = pathEndIndex - pathStartIndex + 1;
		memmove(_pathPolyRefs, _pathPolyRefs + pathStartIndex, _polyLength * sizeof(dtPolyRef));
	}
	else
	{
		printf("maps.mmaps ++ BuildPolyPath :: (!startPolyFound && !endPolyFound)\n");

		// either we have no path at all -> first run
		// or something went really wrong -> we aren't moving along the path to the target
		// just generate new path

		// free and invalidate old path data
		Clear();

		dtStatus dtResult;
		dtResult = _navMeshQuery->findPath(
			startPoly,          // start polygon
			endPoly,            // end polygon
			startPoint,         // start position
			endPoint,           // end position
			&_filter,           // polygon search filter
			_pathPolyRefs,     // [out] path
			(int*)&_polyLength,
			MAX_PATH_LENGTH);   // max number of polygons in output path


		if (!_polyLength || dtStatusFailed(dtResult))
		{
			// only happens if we passed bad data to findPath(), or navmesh is messed up
			printf("maps.mmaps Path Build failed: 0 length path\n");
			BuildShortcut();
			_type = PATHFIND_NOPATH;
			return;
		}
	}

	// by now we know what type of path we can get
	if (_pathPolyRefs[_polyLength - 1] == endPoly && !(_type & PATHFIND_INCOMPLETE))
		_type = PATHFIND_NORMAL;
	else
		_type = PATHFIND_INCOMPLETE;

	AddFarFromPolyFlags(startFarFromPoly, endFarFromPoly);

	// generate the point-path out of our up-to-date poly-path
	BuildPointPath(startPoint, endPoint);
}

void PathGenerator::BuildPointPath(const float_t* startPoint, const float_t* endPoint)
{
	float_t pathPoints[MAX_POINT_PATH_LENGTH * VERTEX_SIZE];
	uint32_t pointCount = 0;
	dtStatus dtResult = DT_FAILURE;

	dtResult = FindSmoothPath(
		startPoint,         // start position
		endPoint,           // end position
		_pathPolyRefs,     // current path
		_polyLength,       // length of current path
		pathPoints,         // [out] path corner points
		(int*)&pointCount,
		_pointPathLimit);    // maximum number of points


	// Special case with start and end positions very close to each other
	if (_polyLength == 1 && pointCount == 1)
	{
		// First point is start position, append end position
		dtVcopy(&pathPoints[1 * VERTEX_SIZE], endPoint);
		pointCount++;
	}
	else if (pointCount < 2 || dtStatusFailed(dtResult))
	{
		printDtStatus(dtResult);

		// only happens if pass bad data to findStraightPath or navmesh is broken
		// single point paths can be generated here
		/// @todo check the exact cases
		printf("maps.mmaps ++ PathGenerator::BuildPointPath FAILED! path sized %d returned, dtResult: %u\n", pointCount, dtResult);
		// BuildShortcut();
		_type = PathType(_type | PATHFIND_NOPATH);
		// return;
	}
	else if (pointCount >= _pointPathLimit)
	{
		printf("maps.mmaps ++ PathGenerator::BuildPointPath FAILED! path sized %d returned, lower than limit set to %d\n", pointCount, _pointPathLimit);
		BuildShortcut();
		_type = PathType(_type | PATHFIND_SHORT);
		return;
	}

	_pathPoints.resize(pointCount);
	for (uint32_t i = 0; i < pointCount; ++i)
		_pathPoints[i] = Vector3(pathPoints[i * VERTEX_SIZE + 2], pathPoints[i * VERTEX_SIZE], pathPoints[i * VERTEX_SIZE + 1]);


	// first point is always our current location - we need the next one
	SetActualEndPosition(_pathPoints[pointCount - 1]);

	// force the given destination, if needed
	if (_forceDestination &&
		(!(_type & PATHFIND_NORMAL) || !InRange(GetEndPosition(), GetActualEndPosition(), 1.0f, 1.0f)))
	{
		// we may want to keep partial subpath
		if (Dist3DSqr(GetActualEndPosition(), GetEndPosition()) < 0.3f * Dist3DSqr(GetStartPosition(), GetEndPosition()))
		{
			SetActualEndPosition(GetEndPosition());
			_pathPoints[_pathPoints.size() - 1] = GetEndPosition();
		}
		else
		{
			SetActualEndPosition(GetEndPosition());
			BuildShortcut();
		}

		_type = PathType(PATHFIND_NORMAL | PATHFIND_NOT_USING_PATH);
	}

	printf("maps.mmaps ++ PathGenerator::BuildPointPath path type %d size %d poly-size %d\n", _type, pointCount, _polyLength);
}

void PathGenerator::BuildShortcut()
{
	printf("maps.mmaps ++ BuildShortcut :: making shortcut\n");
	Clear();

	_pathPoints.resize(2);
	_pathPoints[0] = GetStartPosition();
	_pathPoints[1] = GetActualEndPosition();
	_type = PATHFIND_SHORTCUT;
}

void PathGenerator::CreateFilter()
{
	uint16_t includeFlags = 0;
	uint16_t excludeFlags = 0;
	includeFlags |= (NAV_GROUND | NAV_WATER | NAV_MAGMA_SLIME);
	_filter.setIncludeFlags(includeFlags);
	_filter.setExcludeFlags(excludeFlags);
}

NavTerrainFlag PathGenerator::GetNavTerrain(float_t x, float_t y, float_t z)
{
	return NAV_GROUND;
}

bool PathGenerator::HaveTile(const Vector3& p) const
{
	int tx = -1, ty = -1;
	float_t point[VERTEX_SIZE] = { p.y, p.z, p.x };

	_navMesh->calcTileLoc(point, &tx, &ty);

	/// Workaround
	/// For some reason, often the tx and ty variables wont get a valid value
	/// Use this check to prevent getting negative tile coords and crashing on getTileAt
	if (tx < 0 || ty < 0)
		return false;

	return (_navMesh->getTileAt(tx, ty, 0) != nullptr);
}

uint32_t PathGenerator::FixupCorridor(dtPolyRef* path, uint32_t npath, uint32_t maxPath, dtPolyRef const* visited, uint32_t nvisited)
{
	int32_t furthestPath = -1;
	int32_t furthestVisited = -1;

	// Find furthest common polygon.
	for (int32_t i = npath - 1; i >= 0; --i)
	{
		bool found = false;
		for (int32_t j = nvisited - 1; j >= 0; --j)
		{
			if (path[i] == visited[j])
			{
				furthestPath = i;
				furthestVisited = j;
				found = true;
			}
		}
		if (found)
			break;
	}

	// If no intersection found just return current path.
	if (furthestPath == -1 || furthestVisited == -1)
		return npath;

	// Concatenate paths.

	// Adjust beginning of the buffer to include the visited.
	uint32_t req = nvisited - furthestVisited;
	uint32_t orig = uint32_t(furthestPath + 1) < npath ? furthestPath + 1 : npath;
	uint32_t size = npath > orig ? npath - orig : 0;
	if (req + size > maxPath)
		size = maxPath - req;

	if (size)
		memmove(path + req, path + orig, size * sizeof(dtPolyRef));

	// Store visited
	for (uint32_t i = 0; i < req; ++i)
		path[i] = visited[(nvisited - 1) - i];

	return req + size;
}

bool PathGenerator::GetSteerTarget(float_t const* startPos, float_t const* endPos,
	float_t minTargetDist, dtPolyRef const* path, uint32_t pathSize,
	float_t* steerPos, unsigned char& steerPosFlag, dtPolyRef& steerPosRef)
{
	// Find steer target.
	static const uint32_t MAX_STEER_POINTS = 3;
	float_t steerPath[MAX_STEER_POINTS * VERTEX_SIZE];
	unsigned char steerPathFlags[MAX_STEER_POINTS];
	dtPolyRef steerPathPolys[MAX_STEER_POINTS];
	uint32_t nsteerPath = 0;
	dtStatus dtResult = _navMeshQuery->findStraightPath(startPos, endPos, path, pathSize,
		steerPath, steerPathFlags, steerPathPolys, (int*)&nsteerPath, MAX_STEER_POINTS);
	if (!nsteerPath || dtStatusFailed(dtResult))
		return false;

	// Find vertex far enough to steer to.
	uint32_t ns = 0;
	while (ns < nsteerPath)
	{
		// Stop at Off-Mesh link or when point is further than slop away.
		if ((steerPathFlags[ns] & DT_STRAIGHTPATH_OFFMESH_CONNECTION) ||
			!InRangeYZX(&steerPath[ns * VERTEX_SIZE], startPos, minTargetDist, 1000.0f))
			break;
		ns++;
	}
	// Failed to find good point to steer to.
	if (ns >= nsteerPath)
		return false;

	dtVcopy(steerPos, &steerPath[ns * VERTEX_SIZE]);
	steerPos[1] = startPos[1];  // keep Z value
	steerPosFlag = steerPathFlags[ns];
	steerPosRef = steerPathPolys[ns];

	return true;
}

dtStatus PathGenerator::FindSmoothPath(float_t const* startPos, float_t const* endPos,
	dtPolyRef const* polyPath, uint32_t polyPathSize,
	float_t* smoothPath, int* smoothPathSize, uint32_t maxSmoothPathSize)
{
	*smoothPathSize = 0;
	uint32_t nsmoothPath = 0;

	dtPolyRef polys[MAX_PATH_LENGTH];
	memcpy(polys, polyPath, sizeof(dtPolyRef) * polyPathSize);
	uint32_t npolys = polyPathSize;

	float_t iterPos[VERTEX_SIZE], targetPos[VERTEX_SIZE];

	if (polyPathSize > 1)
	{
		dtStatus status = _navMeshQuery->closestPointOnPolyBoundary(polys[0], startPos, iterPos);
		// Pick the closest points on poly border
		if (dtStatusFailed(status)) {
			printDtStatus(status);
			return DT_FAILURE;
		}

		status = _navMeshQuery->closestPointOnPolyBoundary(polys[npolys - 1], endPos, targetPos);
		if (dtStatusFailed(status)) {
			printDtStatus(status);
			return DT_FAILURE;
		}
	}
	else
	{
		// Case where the path is on the same poly
		dtVcopy(iterPos, startPos);
		dtVcopy(targetPos, endPos);
	}

	dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
	nsmoothPath++;

	// Move towards target a small advancement at a time until target reached or
	// when ran out of memory to store the path.
	while (npolys && nsmoothPath < maxSmoothPathSize)
	{
		// Find location to steer towards.
		float_t steerPos[VERTEX_SIZE];
		unsigned char steerPosFlag;
		dtPolyRef steerPosRef = INVALID_POLYREF;

		if (!GetSteerTarget(iterPos, targetPos, SMOOTH_PATH_SLOP, polys, npolys, steerPos, steerPosFlag, steerPosRef))
			break;

		bool endOfPath = (steerPosFlag & DT_STRAIGHTPATH_END) != 0;
		bool offMeshConnection = (steerPosFlag & DT_STRAIGHTPATH_OFFMESH_CONNECTION) != 0;

		// Find movement delta.
		float_t delta[VERTEX_SIZE];
		dtVsub(delta, steerPos, iterPos);
		float_t len = dtMathSqrtf(dtVdot(delta, delta));
		// If the steer target is end of path or off-mesh link, do not move past the location.
		if ((endOfPath || offMeshConnection) && len < SMOOTH_PATH_STEP_SIZE)
			len = 1.0f;
		else
			len = SMOOTH_PATH_STEP_SIZE / len;

		float_t moveTgt[VERTEX_SIZE];
		dtVmad(moveTgt, iterPos, delta, len);

		// Move
		float_t result[VERTEX_SIZE];
		const static uint32_t MAX_VISIT_POLY = 16;
		dtPolyRef visited[MAX_VISIT_POLY];

		uint32_t nvisited = 0;
		dtStatus status = _navMeshQuery->moveAlongSurface(polys[0], iterPos, moveTgt, &_filter, result, visited, (int*)&nvisited, MAX_VISIT_POLY);
		if (dtStatusFailed(status)) {
			printDtStatus(status);
			return DT_FAILURE;
		}
		npolys = FixupCorridor(polys, npolys, MAX_PATH_LENGTH, visited, nvisited);


		if (dtStatusFailed(_navMeshQuery->getPolyHeight(polys[0], result, &result[1])))
			printf("maps.mmaps Cannot find height at position X: %f Y: %f Z: %f", result[2], result[0], result[1]);
		result[1] += 0.5f;
		dtVcopy(iterPos, result);

		// Handle end of path and off-mesh links when close enough.
		if (endOfPath && InRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 1.0f))
		{
			// Reached end of path.
			dtVcopy(iterPos, targetPos);
			if (nsmoothPath < maxSmoothPathSize)
			{
				dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
				nsmoothPath++;
			}
			break;
		}
		else if (offMeshConnection && InRangeYZX(iterPos, steerPos, SMOOTH_PATH_SLOP, 1.0f))
		{
			// Advance the path up to and over the off-mesh connection.
			dtPolyRef prevRef = INVALID_POLYREF;
			dtPolyRef polyRef = polys[0];
			uint32_t npos = 0;
			while (npos < npolys && polyRef != steerPosRef)
			{
				prevRef = polyRef;
				polyRef = polys[npos];
				npos++;
			}

			for (uint32_t i = npos; i < npolys; ++i)
				polys[i - npos] = polys[i];

			npolys -= npos;

			// Handle the connection.
			float_t connectionStartPos[VERTEX_SIZE], connectionEndPos[VERTEX_SIZE];
			status = _navMesh->getOffMeshConnectionPolyEndPoints(prevRef, polyRef, connectionStartPos, connectionEndPos);
			if (dtStatusSucceed(status))
			{
				if (nsmoothPath < maxSmoothPathSize)
				{
					dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], connectionStartPos);
					nsmoothPath++;
				}
				// Move position at the other side of the off-mesh link.
				dtVcopy(iterPos, connectionEndPos);
				status = _navMeshQuery->getPolyHeight(polys[0], iterPos, &iterPos[1]);
				if (dtStatusFailed(status)) {
					printDtStatus(status);
					return status;
				}
				iterPos[1] += 0.5f;
			}
			else {
				printDtStatus(status);
			}
		}

		// Store results.
		if (nsmoothPath < maxSmoothPathSize)
		{
			dtVcopy(&smoothPath[nsmoothPath * VERTEX_SIZE], iterPos);
			nsmoothPath++;
		}
	}
	*smoothPathSize = nsmoothPath;

	// this is most likely a loop
	return nsmoothPath < MAX_POINT_PATH_LENGTH ? DT_SUCCESS : DT_FAILURE;
}

bool PathGenerator::InRangeYZX(float_t const* v1, float_t const* v2, float_t r, float_t h) const
{
	float_t dx = v2[0] - v1[0];
	float_t dy = v2[1] - v1[1]; // elevation
	float_t dz = v2[2] - v1[2];
	return (dx * dx + dz * dz) < r * r && fabsf(dy) < h;
}

bool PathGenerator::InRange(Vector3 const& p1, Vector3 const& p2, float_t r, float_t h) const
{
	Vector3 d = p1 - p2;
	return (d.x * d.x + d.y * d.y) < r * r && fabsf(d.z) < h;
}

float_t PathGenerator::Dist3DSqr(Vector3 const& p1, Vector3 const& p2) const
{
	return (p1 - p2).squaredLength();
}

void PathGenerator::AddFarFromPolyFlags(bool startFarFromPoly, bool endFarFromPoly)
{
	if (startFarFromPoly)
		_type = PathType(_type | PATHFIND_FARFROMPOLY_START);
	if (endFarFromPoly)
		_type = PathType(_type | PATHFIND_FARFROMPOLY_END);
}
