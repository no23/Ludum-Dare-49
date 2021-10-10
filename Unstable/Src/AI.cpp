#include "AI.h"
#include <vector>
#include <queue>
#include <unordered_map>
#include "Engine.h"
using namespace Unstable;

void Unstable::AI::Init(Engine* engine)
{
	_engine = engine;
}
void Unstable::AI::GenerateNavMesh()
{
	if (_navmesh == nullptr)
	{
		_navmesh = (NavMesh*)calloc(1, sizeof(NavMesh));
	}
	b2World* pWorld = _engine->Physics->world;

	//min of 10x10
	float minWorldX = round(Utility::Min(_engine->minX, -100));
	float minWorldY = round(Utility::Min(_engine->minY, -100));

	float maxWorldX = round(Utility::Max(_engine->maxX, 100));
	float maxWorldY = round(Utility::Max(_engine->maxY, 100));

	float worldTotalX = abs(minWorldX - maxWorldX);
	float worldTotalY = abs(minWorldY - maxWorldY);

	sf::Vector2f wb[] = { { minWorldX, minWorldY }, { maxWorldX, maxWorldY } };
	//sf::Vector2f wb[] = { { -100, -100 }, { 100, 100} };

	//printf("generating nav mesh size:%f %f\nBounds: min %f %f max %f %f\n", worldTotalX, worldTotalY, _engine->minX, _engine->minY, _engine->maxX, _engine->maxY);

	_navmesh->WorldBounds[0] = wb[0];
	_navmesh->WorldBounds[1] = wb[1];

	float cellSize = 10;
	int sx = worldTotalX / cellSize;
	int sy = worldTotalY / cellSize;

	_navmesh->GridSize = cellSize;

	if (_navmesh->Data == nullptr)
	{
		_navmesh->Data = (s32*)calloc(sx * sy, sizeof(s32));
		_navmesh->GridWidth = sx;
		_navmesh->GridHeight = sy;
	}

	if (sx != _navmesh->GridWidth || sy != _navmesh->GridHeight)
	{
		_navmesh->Data = (s32*)realloc(_navmesh->Data, sx * sy * sizeof(s32));
		assert(_navmesh->Data != nullptr);

		_navmesh->GridWidth = sx;
		_navmesh->GridHeight = sy;
	}

	for (s32 y = 0; y < sy; y++)
	{
		for (s32 x = 0; x < sx; x++)
		{
			float wx = (x * cellSize) + wb[0].x;
			float wy = (y * cellSize) + wb[0].y;

			float cwx = wx + (cellSize * 0.5f);
			float cwy = wy + (cellSize * 0.5f);

			s32 index = x + (y * sx);
			_navmesh->Data[index] = 0;

			if (Physics::RayIntersect(sf::Vector2f(cwx, cwy), sf::Vector2f(cellSize, cellSize)))
			{
				_navmesh->Data[index] = 1;
			}
		}
	}

	if (_engine->ShowPlayerDebug)
	{
		for (s32 y = 0; y < sy; y++)
		{
			for (s32 x = 0; x < sx; x++)
			{
				float wx = (x * cellSize) + wb[0].x;
				float wy = (y * cellSize) + wb[0].y;

				float cwx = wx + (cellSize * 0.5f);
				float cwy = wy + (cellSize * 0.5f);

				s32 index = x + (y * sx);

				if (_navmesh->Data[index] > 0)
				{
					Debug::DrawBox(sf::Vector2f(cwx, cwy), sf::Vector2f(cellSize, cellSize), false);
				}
			}
		}
	}
}

sf::Vector2i Unstable::AI::WorldToGrid(sf::Vector2f point)
{
	int tlx = point.x - _navmesh->WorldBounds[0].x;
	int tly = point.y - _navmesh->WorldBounds[0].y;

	int x = tlx / _navmesh->GridSize;
	int y = tly / _navmesh->GridSize;
	return sf::Vector2i(x, y);
}

std::vector<sf::Vector2i> Unstable::AI::GetNeighbors(sf::Vector2i point)
{
	std::vector<sf::Vector2i> points;

	const sf::Vector2i directions[] = {
		sf::Vector2i(0,-1),
		sf::Vector2i(1,0),
		sf::Vector2i(0,1),
		sf::Vector2i(-1,0),
	};

	for (s32 i = 0; i < 4; i++)
	{
		sf::Vector2i n = directions[i] + point;
		if (n.x >= 0 && n.x < _navmesh->GridWidth && n.y >= 0 && n.y < _navmesh->GridHeight)
		{
			s32 index = point.x + point.y * _navmesh->GridWidth;

			if (_navmesh->Data[index] == 0)
			{
				points.push_back(n);
			}
		}
	}

	return points;
}

namespace std {
	/* implement hash function so we can put GridLocation into an unordered_set */
	template <> struct hash<sf::Vector2i> {
		std::size_t operator()(const sf::Vector2i& id) const noexcept {
			return std::hash<int>()(id.x ^ (id.y << 4));
		}
	};
}

int h(sf::Vector2i v0, sf::Vector2i v1)
{
	return std::abs(v0.x - v1.x) + std::abs(v0.y - v1.y);
}

std::vector<sf::Vector2f> Unstable::AI::GeneratePath(Engine* engine, sf::Vector2f start, sf::Vector2f end)
{
	struct s {
		sf::Vector2i v;
		int p;

		bool operator>(const s& rhs) const
		{
			return p > rhs.p;
		}
	};

	std::unordered_map<sf::Vector2i, sf::Vector2i> cameFrom;
	std::unordered_map<sf::Vector2i, int> costSoFar;

	std::priority_queue<s, std::vector<s>, std::greater<s>> open;

	sf::Vector2i gridStart = WorldToGrid(start);
	sf::Vector2i gridEnd = WorldToGrid(end);

	open.push({ gridStart, 0 });
	cameFrom[gridStart] = gridStart;
	costSoFar[gridStart] = 0;

	while (open.size() > 0)
	{
		sf::Vector2i loc = open.top().v;
		open.pop();

		if (loc == gridEnd)
		{
			break;
		}

		for (sf::Vector2i next : GetNeighbors(loc))
		{
			s32 cindex = next.x + next.y * _navmesh->GridWidth;
			f32 nc = costSoFar[loc] + 5;

			if (costSoFar.find(next) == costSoFar.end() || nc < costSoFar[next])
			{
				costSoFar[next] = nc;
				int p = nc + h(next, gridEnd);
				open.push({ next, p });
				cameFrom[next] = loc;
			}
		}
	}

	std::vector<sf::Vector2f> path;
	sf::Vector2i current = gridEnd;
	while (current != gridStart)
	{
		if (path.size() > cameFrom.size())
		{
			return std::vector<sf::Vector2f>();
		}
		path.push_back({ (f32)current.x * _navmesh->GridSize + _navmesh->WorldBounds[0].x + (_navmesh->GridSize / 2), (f32)current.y * _navmesh->GridSize + _navmesh->WorldBounds[0].y + (_navmesh->GridSize / 2) });
		current = cameFrom[current];
	}

	std::reverse(path.begin(), path.end());
	return path;
}