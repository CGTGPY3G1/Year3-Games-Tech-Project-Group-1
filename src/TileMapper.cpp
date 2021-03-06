#include "TileMapper.h"
#include <queue>
#include <map>
#include "AssetManager.h"
#include "Engine.h"
#include "FileManager.h"
#include "GameObjectFactory.h"
#include <string>
#include "PlayerScript.h"
#include "PhysicsSystem.h"
namespace WeiSungEngine {
	TileMapper::TileMapper() : Component() {

	}

	TileMapper::TileMapper(std::weak_ptr<GameObject> gameObject) : Component(gameObject) {

	}

	TileMapper::~TileMapper() {
	}

	void TileMapper::Init(const std::string & fileName, const std::string & tilesetName) {
		std::string tmx = FileManager::LoadTMX(fileName);
		LoadTmxMap(tmx, tilesetName);
	}

	bool TileMapper::LoadTmxMap(const std::string & xml, const std::string & tilesetName) {
		map = std::make_shared<TmxMap>(xml);
		std::shared_ptr<TmxGroup> navGroup, characterGroup, vehicleGroup;
		if (map->isValid) {
			std::shared_ptr<GameObject> g = GameObjectManager::GetInstance().CreateGameObject("NavBlocker").lock();
			g->Init();
			std::shared_ptr<RigidBody2D> rb = g->AddComponent<RigidBody2D>().lock();
			rb->Init();
			std::shared_ptr<Transform2D> transform = GetComponent<Transform2D>().lock();
			worldScale = transform->GetScale();
			halfWidth = (float)(map->width * map->tileWidth) * 0.5f;
			halfHeight = (float)(map->height * map->tileHeight) * 0.5f;
			bool foundSet = false;
			std::shared_ptr<TmxTileset> tileset;
			const unsigned int noOfTilesets = map->tilesets.size();
			for (size_t i = 0; i < noOfTilesets && !foundSet; i++) {
				if (tilesetName == map->tilesets[i]->name) {
					tileset = map->tilesets[i];
					foundSet = true;
				}
			}
			sf::Texture & texture = AssetManager::GetInstance().GetTexture("Tilesets/" + tileset->filename);
			const unsigned int noOfLayers = map->layers.size();
			for (size_t j = 0; j < noOfLayers; j++) {
				std::shared_ptr<TmxLayer> layer = map->layers[j];
				layer->name;
				if (layer->name == "BackgroundLayer") ProcessTmxTileLayer(tileset, layer, texture); ;
			}
			const unsigned int noOfGroups = map->groups.size();
			for (size_t k = 0; k < noOfGroups; k++) {
				std::shared_ptr<TmxGroup> group = map->groups[k];
				group->name;
				if (group->name == "Buildings") ProcessTmxBuildingGroup(group);
				else if (group->name == "Characters") characterGroup = group;
				else if (group->name == "Vehicles") vehicleGroup = group;
				else if (group->name == "Barriers") ProcessTmxBarrierGroup(group);
				else if (group->name == "NavigationLayer") navGroup = group;
			}
			if (navGroup) ProcessNavigationLayer(navGroup);
			if (vehicleGroup) ProcessVehicles(vehicleGroup);
			if (characterGroup) ProcessCharacters(characterGroup);
			g->Destroy();
		}



		return map->isValid;
	}



	void TileMapper::ProcessTmxBuildingGroup(std::shared_ptr<TmxGroup> group) {
		const size_t noOfObjects = group->objects.size();

		for (size_t i = 0; i < noOfObjects; i++) {
			std::shared_ptr<TmxObject> object = group->objects[i];
			const size_t noOfProperties = object->properties.size();
			const Vector2 objectScale = GetObjectScale(0, object->type, object->width, object->height);
			const Vector2 adjustedScale = Vector2(worldScale.x * objectScale.x, worldScale.y * objectScale.y);
			Vector2 position = Vector2((object->x - halfWidth) * worldScale.x, (object->y - halfHeight) * worldScale.y) + Vector2((object->width / 2) * worldScale.x, (object->height / 2) * -worldScale.y).RotateInDegrees(object->rotation);
			std::shared_ptr<GameObject> newBuilding = GameObjectFactory::CreateBuilding(object->type, position, adjustedScale, object->rotation).lock();
			std::vector<std::weak_ptr<BoxCollider>> colliders = newBuilding->GetComponents<BoxCollider>();
			std::shared_ptr<Transform2D> transform = newBuilding->GetComponent<Transform2D>().lock();
			for (std::vector<std::weak_ptr<BoxCollider>>::iterator it = colliders.begin(); it != colliders.end(); ++it) {
				std::shared_ptr<BoxCollider> bc = (*it).lock();
				if (bc) {
					const AABB aabb = bc->GetAABB();
					Vector2 buffer = Vector2(16.0f, 16.0f);
					Vector2 worldTopLeft = Vector2(aabb.left + buffer.x, aabb.top - buffer.y);
					Vector2 worldBottomRight = Vector2(aabb.right - buffer.x, aabb.bottom + buffer.y);
					GridLocation topLeft = WorldToGrid(worldTopLeft), bottomRight = WorldToGrid(worldBottomRight);
					const int tilesHigh = tiles.size() - 1, tilesWide = tiles[0].size() - 1;
					const int minY = std::max<int>(0, std::min<int>(bottomRight.y, tilesHigh));
					const int maxY = std::min<int>(tilesHigh, std::max<int>(0, topLeft.y));
					const int minX = std::max<int>(0, std::min<int>(topLeft.x, tilesWide));
					const int maxX = std::min<int>(tilesWide, std::max<int>(0, bottomRight.x));

					for (unsigned int j = minY; j <= maxY; j++) {
						for (unsigned int k = minX; k <= maxX; k++) {
							tiles[j][k].type = TileType::TILE_TYPE_NULL;
						}
					}
				}
			}
		}
	}

	void TileMapper::ProcessTmxBarrierGroup(std::shared_ptr<TmxGroup> group) {
		const size_t noOfObjects = group->objects.size();
		for (size_t i = 0; i < noOfObjects; i++) {
			std::shared_ptr<TmxObject> object = group->objects[i];
			const Vector2 objectScale = Vector2(object->width / 32.0f, object->height / 32.0f);
			const Vector2 adjustedScale = Vector2(worldScale.x * objectScale.x, worldScale.y * objectScale.y);
			Vector2 position = Vector2((object->x - halfWidth) * worldScale.x, (object->y - halfHeight) * worldScale.y) + Vector2((object->width / 2) * worldScale.x, (object->height / 2) * -worldScale.y).RotateInDegrees(object->rotation);
			std::shared_ptr<GameObject> newBarrier = GameObjectFactory::CreateBarrier(object->type, position, adjustedScale, object->rotation).lock();
		}
	}

	void TileMapper::ProcessTmxTileLayer(std::shared_ptr<TmxTileset> tileset, std::shared_ptr<TmxLayer> layer, sf::Texture & texture) {
		const std::vector<int> data = layer->data;
		const int noOfTiles = (int)data.size();
		width = layer->width; height = layer->height;
		const int sourceColumns = (tileset->sourceWidth / tileset->tileWidth);
		const int tileHalfWidth = tileWidth >> 1, tileHalfHeight = tileHeight >> 1;
		std::shared_ptr<GameObject> g = GameObjectManager::GetInstance().GetGameObject("NavBlocker").lock();
		Vector2 offset = Vector2(Physics::PIXELS_PER_METRE, Physics::PIXELS_PER_METRE);
		for (int i = 0; i < noOfTiles; i++) {
			const int gid = layer->data[i];
			const int x = (gid - 1) % sourceColumns;
			const int y = (gid - 1) / sourceColumns;
			const int ax = (i % width), ay = (i / width);

			if (ax == 0) tiles.push_back(std::vector<Tile>());

			sf::IntRect rect = sf::IntRect((x*(tileset->tileWidth + 2) + 1), (y*(tileset->tileHeight + 2) + 1), tileset->tileWidth, tileset->tileHeight);
			sf::Sprite sprite(texture, rect);
			Vector2 position = Vector2(-halfWidth + (ax * tileset->tileWidth), -halfHeight + (ay * tileset->tileWidth));
			sprite.setPosition(position);

			Tile toAdd;
			toAdd.verts.push_back(sf::Vertex(position, sf::Vector2f((float)rect.left, (float)rect.top)));
			toAdd.verts.push_back(sf::Vertex(sf::Vector2f(position.x + tileset->tileWidth, position.y), sf::Vector2f((float)(rect.left + rect.width), (float)rect.top)));
			toAdd.verts.push_back(sf::Vertex(sf::Vector2f(position.x + tileset->tileWidth, position.y + tileset->tileHeight), sf::Vector2f((float)(rect.left + rect.width), (float)(rect.top + rect.height))));
			toAdd.verts.push_back(sf::Vertex(sf::Vector2f(position.x, position.y + tileset->tileHeight), sf::Vector2f((float)rect.left, (float)(rect.top + rect.height))));
			toAdd.sprite = sprite;
			toAdd.worldPosition = (position * 4.0f) + (offset * 0.5f);
			toAdd.id = gid;
			TileType type = TypeFromGID(gid);
			toAdd.type = type;
			tiles[ay].push_back(toAdd);

			if (type == TileType::TILE_TYPE_NULL || type == TileType::TILE_TYPE_WATER || type == TileType::TILE_TYPE_ROAD) {
				std::shared_ptr<BoxCollider> bc = g->AddComponent<BoxCollider>().lock();
				bc->Init(toAdd.worldPosition, offset, true);
			}
		}
	}

	void TileMapper::ProcessCharacters(std::shared_ptr<TmxGroup> group) {
		const size_t noOfObjects = group->objects.size();
		for (size_t i = 0; i < noOfObjects; i++) {
			std::shared_ptr<TmxObject> object = group->objects[i];
			const size_t noOfProperties = object->properties.size();
			Vector2 position = Vector2((object->x - halfWidth)* worldScale.x, (object->y - halfHeight) * worldScale.y) + Vector2((object->width / 2) * worldScale.x, (object->height / 2) * -worldScale.y).RotateInDegrees(object->rotation);
			if (object->name == "Player") GameObjectFactory::CreateCharacter(object->name, object->type, false, position, Vector2::One, object->rotation).lock();
			else GameObjectFactory::CreateCharacter(object->name, object->type, true, position, Vector2::One, object->rotation).lock();
		}
	}

	void TileMapper::ProcessVehicles(std::shared_ptr<TmxGroup> group) {

		const size_t noOfObjects = group->objects.size();

		for (size_t i = 0; i < noOfObjects; i++) {
			std::shared_ptr<TmxObject> object = group->objects[i];

			const size_t noOfProperties = object->properties.size();
			Vector2 position = Vector2((object->x - halfWidth)* worldScale.x, (object->y - halfHeight) * worldScale.y) + Vector2((object->width / 2) * worldScale.x, (object->height / 2) * -worldScale.y).RotateInDegrees(object->rotation);
			GameObjectFactory::CreateVehicle(object->type, position, Vector2::One, object->rotation).lock();
		}
	}

	void TileMapper::ProcessNavigationLayer(std::shared_ptr<TmxGroup> group) {
		const size_t noOfObjects = group->objects.size();
		std::vector<std::shared_ptr<NavInfo>> nodes;
		for (size_t i = 0; i < noOfObjects; i++) {
			std::shared_ptr<TmxObject> object = group->objects[i];
			Vector2 position = Vector2((object->x - halfWidth) * worldScale.x, (object->y - halfHeight) * worldScale.y) + Vector2((object->width / 2) * worldScale.x, (object->height / 2) * -worldScale.y);
			GridLocation g = WorldToGrid(position);
			std::shared_ptr<NavInfo> node = std::make_shared<NavInfo>(position, g);
			tiles[g.y][g.x].navInfo = node;
			tiles[g.y][g.x].closestNode = g;
			nodes.push_back(node);
		}
		for (std::vector<std::shared_ptr<NavInfo>>::iterator i = nodes.begin(); i != nodes.end() - 1;) {
			for (std::vector<std::shared_ptr<NavInfo>>::iterator j = i + 1; j != nodes.end();) {
				if ((*i)->gridLocation.x == (*j)->gridLocation.x && (*i)->gridLocation.y == (*j)->gridLocation.y) {
					i = nodes.erase(i);
				}
				else {
					++j;  ++i;
				}
			}
		}

		const float maxDistance = (Physics::PIXELS_PER_METRE * 50.0f) * (Physics::PIXELS_PER_METRE * 50.0f);
		const size_t noOfNodes = nodes.size();
		for (size_t i = 0; i < noOfNodes - 1; i++) {
			std::shared_ptr<NavInfo> n1 = nodes[i];
			for (size_t j = i + 1; j < noOfNodes; j++) {
				std::shared_ptr<NavInfo> n2 = nodes[j];
				if (!(n1 && n2) || n1 == n2)
					continue;
				if ((n2->worldPosition - n1->worldPosition).SquareMagnitude() < maxDistance) {
					RayCastHit hit = PhysicsSystem::GetInstance().RayCast(n1->worldPosition, n2->worldPosition, true);
					if (!hit.hit) {
						Edge e1, e2;
						e1.linked = n2;
						e2.linked = n1;
						bool alreadyLinked1 = n1->AddEdge(e1);
						bool alreadyLinked2 = n2->AddEdge(e2);
					}
				}
			}
		}
		int iteration = 1;
		bool done = false;
		while (nodes.size() > 0 && !done) {
			const GridLocation nullGridLocation = GridLocation(-1, -1);
			bool stillProcessing = false;
			for (std::vector<std::shared_ptr<NavInfo>>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
				if ((*i)->processed) continue;
				if (!stillProcessing) stillProcessing = true;
				const GridLocation nodeLocation = (*i)->gridLocation;
				int y = nodeLocation.y, x = nodeLocation.x;
				bool foundEmpty = false;
				for (size_t dx = iteration; dx > 0; dx--) {
					int locX = x + dx, locY = y - (iteration - dx);
					if (IsOnGrid(locX, locY)) {
						if (tiles[locY][locX].closestNode == nullGridLocation) {
							tiles[locY][locX].closestNode = nodeLocation;
							if (!foundEmpty) foundEmpty = true;
						}
					}
					locX = x - dx, locY = y + (iteration - dx);
					if (IsOnGrid(locX, locY)) {
						if (tiles[locY][locX].closestNode == nullGridLocation) {
							tiles[locY][locX].closestNode = nodeLocation;
							if (!foundEmpty) foundEmpty = true;
						}
					}
					locX = x - (iteration - dx);  locY = y - dx;
					if (IsOnGrid(locX, locY)) {
						if (tiles[locY][locX].closestNode == nullGridLocation) {
							tiles[locY][locX].closestNode = nodeLocation;
							if (!foundEmpty) foundEmpty = true;
						}
					}
					locX = x + (iteration - dx);  locY = y + dx;
					if (IsOnGrid(locX, locY)) {
						if (tiles[locY][locX].closestNode == nullGridLocation) {
							tiles[locY][locX].closestNode = nodeLocation;
							if (!foundEmpty) foundEmpty = true;
						}
					}
				}
				if (!foundEmpty) (*i)->processed = true;
			}
			iteration++;
			done = !stillProcessing;
		}
	}

	Vector2 TileMapper::IndexToWorld(const int & index) {
		const GridLocation gl = IndexToGrid(index);
		Vector2 toReturn = Vector2(std::round(-halfWidth + (gl.x * tileWidth)), std::round(-halfHeight + (gl.y * tileWidth)));
		return toReturn;
	}

	const GridLocation TileMapper::IndexToGrid(const int & index) {
		return GridLocation(index % width, index / width);
	}

	const Vector2 TileMapper::GridToWorld(const GridLocation & gridLocation) {
		const float x = (((gridLocation.x * tileWidth) - halfWidth + (tileWidth * 0.5f)) * worldScale.x), y = (((gridLocation.y * tileHeight) - halfHeight + (tileHeight * 0.5f)) * worldScale.y);
		return Vector2(x, y);
	}

	bool TileMapper::IsOnGrid(const GridLocation & gridLocation) {
		return IsOnGrid(gridLocation.x, gridLocation.y);
	}

	bool TileMapper::IsOnGrid(const int & x, const int & y) {
		return y < tiles.size() && y >= 0 && x < tiles[0].size() && x >= 0;
	}

	const GridLocation TileMapper::WorldToGrid(const Vector2 & worldPosition) {
		const int x = (int)(((worldPosition.x / worldScale.x) + halfWidth) / (tileWidth)), y = (int)(((worldPosition.y / worldScale.y) + halfHeight) / tileHeight);
		return GridLocation(x, y);
	}

	const TileType TileMapper::GetTileType(const Vector2 & worldPosition) {
		GridLocation toGrid = WorldToGrid(worldPosition);
		if (toGrid.x < 0 || toGrid.x >= width || toGrid.y < 0 || toGrid.y >= height) return TileType::TILE_TYPE_NULL;
		return tiles[toGrid.y][toGrid.x].type;
	}

	const std::string TileMapper::GetTileTypeAsString(const Vector2 & worldPosition) {
		TileType type = GetTileType(worldPosition);
		if (type == TileType::TILE_TYPE_NULL) return "No Tile Detected";
		return tileTypeNames[(int)type];
	}

	const float TileMapper::GetTileForceScale(const Vector2 & worldPosition) {
		TileType type = GetTileType(worldPosition);
		if (type == TileType::TILE_TYPE_NULL) return 1.0f;
		return forceScaleTable[(int)type];
	}

	const Tile & TileMapper::GetTile(const Vector2 & worldPosition) const {
		const int x = (int)(((worldPosition.x / worldScale.x) + halfWidth) / (tileWidth)), y = (int)(((worldPosition.y / worldScale.y) + halfHeight) / tileHeight);
		if (x < 0 || x >= width || y < 0 || y >= height) return Tile();
		return tiles[y][x];
	}

	void TileMapper::PrintTile(const Vector2 & worldPosition) {
		Tile tile = GetTile(worldPosition);
		GridLocation l = WorldToGrid(worldPosition);
		if (tile.navInfo) std::cout << "Y = " + std::to_string(l.y) + " X = " + std::to_string(l.x) << std::endl <<
			"World Position = " + worldPosition.ToString() << std::endl <<
			"Tile Position = " + tile.worldPosition.ToString() << std::endl <<
			"Navinfo Position = " + tile.navInfo->worldPosition.ToString() << std::endl;
	}


	bool TileMapper::IsValid() {
		if (!map) return false;
		return map->isValid;
	}

	void TileMapper::Draw() {
		if (map) {
			std::shared_ptr<Graphics> graphics = Engine::GetInstance().GetGraphics().lock();
			if (!myTransform.lock()) {
				myTransform = GetComponent<Transform2D>();
			}
			std::shared_ptr<Transform2D> transform = myTransform.lock();
			if (transform) {
				Vector2 goPosition = transform->GetPosition();
				Vector2 scale = transform->GetScale();
				sf::Vector2f size = graphics->GetView().getSize(), position = graphics->GetView().getCenter();
				const unsigned int halfWidth = map->width / 2, halfHeight = map->height / 2;
				const int top = (int)(((position.y - goPosition.y - (size.y * 0.5f)) / (scale.y * map->tileHeight)) + halfHeight),
					left = (int)(((position.x - goPosition.x - (size.x * 0.5f)) / (scale.x * map->tileWidth)) + halfWidth - 1),
					bottom = (int)(((position.y - goPosition.y + (size.y * 0.5f)) / (scale.y * map->tileHeight)) + halfHeight + 1),
					right = (int)(((position.x - goPosition.y + (size.x * 0.5f)) / (scale.x * map->tileWidth)) + halfWidth + 1);

				const int tilesHigh = tiles.size(), tilesWide = tiles[0].size();
				const int minY = std::max<int>(0, std::min<int>(top, tilesHigh));
				const int maxY = std::min<int>(tilesHigh, std::max<int>(0, bottom));
				const int minX = std::max<int>(0, std::min<int>(left, tilesWide));
				const int maxX = std::min<int>(tilesWide, std::max<int>(0, right));
				const int drawWidth = std::abs(maxX - minX), drawHeight = std::abs(maxY - minY);


				const size_t arraySize = drawWidth * drawHeight * 4;
				if (arraySize > 0) {
					sf::VertexArray vertexArray(sf::Quads, arraySize);
					sf::RenderStates rs;
					rs.transform = transform->GetWorldTransform();
					bool first = true;

					for (unsigned int i = minY; i < maxY; i++) {
						for (unsigned int j = minX; j < maxX; j++) {
							Tile tile = tiles[i][j];
							// Don't use vertex array
							// graphics->Draw(tile.sprite, rs);
							// Use Vertex array
							if (first) {
								rs.texture = tile.sprite.getTexture();
								first = false;
							}
							for (size_t i = 0; i < tile.verts.size(); i++) {
								vertexArray.append(tile.verts[i]);
							}
						}
					}
					graphics->Draw(vertexArray, rs);
					if (showGridLinks || showNavNodes) {
						for (unsigned int i = minY; i < maxY; i++) {
							for (unsigned int j = minX; j < maxX; j++) {
								Tile tile = tiles[i][j];

								if (showNavNodes && tile.navInfo) {
									for (size_t b = 0; b < tile.navInfo->edges.size(); b++) {
										sf::Vertex line[] = {
											sf::Vertex(tile.navInfo->worldPosition, sf::Color::White),
											sf::Vertex(tile.navInfo->edges[b].linked->worldPosition, sf::Color::White)
										};
										graphics->DrawLine(line);
									}
									graphics->DrawCircle(tile.navInfo->worldPosition, 64.0f);
								}
								if (showGridLinks) {
									Vector2 target = GridToWorld(tile.closestNode);
									sf::Vertex line[] = {
										sf::Vertex(tile.worldPosition, sf::Color::Red),
										sf::Vertex(target, sf::Color::Red)
									};
									graphics->DrawLine(line);
								}
							}
						}
					}

				}
			}
		}
	}

	Vector2 TileMapper::GetNewTargetLocation(const Vector2 & worldPosition) {
		Tile t = GetTile(worldPosition);
		if (t.navInfo) {
			std::shared_ptr<NavInfo> linked = t.navInfo->GetRandomNode().lock();
			if (linked) {
				return linked->worldPosition;
			}
			else {
				return t.navInfo->worldPosition;
			}
		}
		return GridToWorld(t.closestNode);
	}

	TileType TileMapper::TypeFromGID(unsigned int gid) {
		TileType toReturn;
		switch (gid) {
		case 1:
			toReturn = TileType::TILE_TYPE_SAND;
			break;
		case 2:
			toReturn = TileType::TILE_TYPE_WATER;
			break;
		case 3:
			toReturn = TileType::TILE_TYPE_GRASS;
			break;
		case 19: case 20: case 21:  case 37:
			toReturn = TileType::TILE_TYPE_PAVEMENT;
			break;
		case 55:  case 211: case 212: case 213: case 214: case 215: case 216: case 229: case 230:  case 231:
		case 232: case 233: case 234: case 247: case 248: case 249: case 250:  case 251: case 252: case 265:
		case 266: case 267: case 268: case 269: case 270: case 283:  case 284: case 285: case 286: case 287:
		case 288: case 301: case 302: case 303: case 304:  case 305: case 306:
			toReturn = TileType::TILE_TYPE_ROAD;
			break;
		case 209: case 210: case 227: case 245:
			toReturn = TileType::TILE_TYPE_ROAD_DIVIDER;
			break;
		case 4: case 5: case 6:  case 22: case 24: case 40: case 41: case 42:
		case 61: case 62: case 63:  case 79: case 81: case 97: case 98: case 99:
			toReturn = TileType::TILE_TYPE_WATER_SAND;
			break;

		case 7: case 8: case 9: case 25: case 27: case 43: case 44: case 45:
		case 64: case 65: case 66: case 82: case 84: case 100: case 101: case 102:
			toReturn = TileType::TILE_TYPE_SAND_GRASS;
			break;
		case 13: case 14: case 15: case 31: case 33: case 49: case 50: case 51:
		case 58: case 59: case 60: case 76: case 78: case 94: case 95: case 96:
			toReturn = TileType::TILE_TYPE_WATER_GRASS;
			break;
		case 109: case 110: case 111: case 112: case 113: case 114: case 115: case 116:
		case 117: case 118: case 119: case 120: case 127: case 129: case 130: case 132:
		case 133: case 135: case 136: case 138: case 145: case 146: case 147: case 148:
		case 149: case 150: case 151: case 152: case 153: case 154: case 155: case 156:
		case 163: case 164: case 165: case 166: case 167: case 168: case 169: case 170:
		case 171: case 172: case 181: case 182: case 183: case 184: case 185: case 186:
		case 187: case 188: case 189: case 190: case 199: case 200: case 201: case 202:
		case 217: case 218: case 219: case 220: case 235: case 236: case 237: case 238:
		case 253: case 254: case 255: case 256: case 257: case 258: case 271: case 273:
		case 274: case 276: case 289: case 290: case 291: case 292: case 293: case 294:
		case 307: case 308: case 309: case 310: case 311: case 312: case 325: case 327:
		case 328: case 330: case 343: case 344: case 345: case 346: case 347: case 348:
			toReturn = TileType::TILE_TYPE_PAVEMENT_ROAD;
			break;
		case 121: case 122: case 123: case 124: case 125: case 126: case 139: case 141:
		case 142: case 144: case 157: case 158: case 159: case 160: case 161: case 162:
		case 173: case 174: case 175: case 176: case 177: case 178: case 179: case 180:
		case 191: case 192: case 193: case 194: case 195: case 196: case 197: case 198:
		case 203: case 204: case 205: case 206: case 207: case 208: case 221: case 223:
		case 224: case 226: case 239: case 240: case 241: case 242: case 243: case 244:
		case 259: case 260: case 261: case 262: case 263: case 264: case 277: case 279:
		case 280: case 282: case 295: case 296: case 297: case 298: case 299: case 300:
		case 313: case 314: case 315: case 316: case 317: case 318: case 331: case 333:
		case 334: case 336: case 349: case 350: case 351: case 352: case 353: case 354:
			toReturn = TileType::TILE_TYPE_PAVEMENT_GRASS;
			break;
		case 361: case 362: case 363: case 364: case 365: case 366: case 367: case 368:
		case 369: case 370: case 372: case 379: case 381: case 382: case 384: case 385:
		case 387: case 388: case 390: case 397: case 398: case 399: case 400: case 401:
		case 402: case 403: case 404: case 405: case 406: case 407: case 408: case 415:
		case 416: case 417: case 418: case 419: case 420: case 421: case 422: case 433:
		case 434: case 435: case 436: case 437: case 438: case 439: case 440:
			toReturn = TileType::TILE_TYPE_PAVEMENT_WATER;
			break;
		default:
			toReturn = TileType::TILE_TYPE_NULL;
			break;
		}

		return toReturn;
	}

	Vector2 TileMapper::GetObjectScale(const unsigned int & layerType, const unsigned int & objectType, const float & width, const float & height) {
		if (layerType == 0) {
			switch (objectType) {
			case 1:
				return Vector2(width / 64.0f, height / 64.0f);
			case 2:
				return Vector2(width / 192.0f, height / 160.0f);
			case 3:
				return Vector2(width / 192.0f, height / 224.0f);
			case 4:
				return Vector2(width / 128.0f, height / 96.0f);
			case 5:
				return Vector2(width / 224.0f, height / 288.0f);
			case 6:
				return Vector2(width / 128.0f, height / 224.0f);
			case 7:
				return Vector2(width / 256.0f, height / 128.0f);
			case 8:
				return Vector2(width / 160.0f, height / 96.0f);
			case 9:
				return Vector2(width / 256.0f, height / 96.0f);
			case 10:
				return Vector2(width / 96.0f, height / 64.0f);
			case 11:
				return Vector2(width / 96.0f, height / 96.0f);
			case 12:
			case 13:
			case 14:
			case 15:
				return Vector2(width / 160.0f, height / 64.0f);
			case 16:
			case 17:
			case 18:
				return Vector2(width / 128.0f, height / 352.0f);
			case 19:
			case 20:
				return Vector2(width / 192.0f, height / 128.0f);
			default:
				break;
			}
		}
		else {
			// is anything else that's scalable getting added to the editor?
		}
		return Vector2::One;
	}

	std::weak_ptr<NavInfo> NavInfo::GetRandomNode() {
		const int index = Random::RandomInt(0, edges.size());
		return edges[index].linked;
	}
}
