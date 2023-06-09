#pragma once
#include <functional>
#include "wow/Offsets.h"
#include "wow/entity/CGObject.h"
#include "wow/entity/CGGameObject.h"
#include "wow/entity/CGItem.h"

#pragma pack(push, 1)
struct BucketEntry {
	char padding_0x10[0x10];	// 0x00 - 0x10
	CGObject* object;			// 0x10 - 0x18
};

struct CGObjectManager
{
	char padding_0x18[0x18];	// 0x00 - 0x18
	CGObject* head;				// 0x18 - 0x20
	int32_t objectCount;		// 0x20 - 0x24
	char padding_0x30[0x0C];	// 0x24 - 0x30
	BucketEntry* buckets;		// 0x30 - 0x38
	char padding_0x48[0x10];	// 0x38 - 0x48
	int32_t bucketCount;		// 0x48 - 0x4C

	/* EJT of OwnedCore */
	/* ChrisIsMe of OwnedCore */
	/* https://www.ownedcore.com/forums/world-of-warcraft/world-of-warcraft-bots-programs/wow-memory-editing/924897-retail-shadowlands-9-0-1-36230-offsets-2.html */
	CGObject* getByGuid(CGGuid* guid) {
		if (guid == nullptr || guid->isEmpty())
			return nullptr;

		auto k1 = 0xD6D018F5 * static_cast<uint32_t>(guid->high & 0xFFFFFFFF);
		auto k2 = 0x5D55FCC5 * static_cast<uint32_t>(guid->low & 0xFFFFFFFF);
		auto k3 = 0xD6D018F5i64 * guid->high;
		auto k4 = 0xA2AA033Bi64 * guid->low;

		uint32_t hash = static_cast<uint32_t>(k1 - k2 + ((k3 + k4) >> 32));
		auto index = hash % this->bucketCount;

		auto currentObject = (this->buckets + index)->object;
		while (ValidPointer(currentObject))
		{
			if (currentObject->guidHash == hash)
				return currentObject;
			currentObject = currentObject->linked;
		}
		return nullptr;
	}

	void enumerateObjects(const std::function<void(CGObject*)>& callback)
	{
		auto currentObject = this->head;
		while (ValidPointer(currentObject))
		{
			callback(currentObject);
			currentObject = currentObject->next;
		}
	}

	CGGameObject* findGameObject(const std::function<bool(CGGameObject*)>& filter)
	{
		auto currentObject = this->head;
		while (ValidPointer(currentObject))
		{
			if (currentObject->type == ObjectType::GameObject)
			{
				CGGameObject* gameObject = reinterpret_cast<CGGameObject*>(currentObject);
				if (filter(gameObject))
					return gameObject;
			}

			currentObject = currentObject->next;
		}

		return nullptr;
	}

	CGItem* findItem(const std::function<bool(CGItem*)>& filter)
	{
		auto currentObject = this->head;
		while (ValidPointer(currentObject))
		{
			if (currentObject->type == ObjectType::Item)
			{
				CGItem* item = reinterpret_cast<CGItem*>(currentObject);
				if (filter(item))
					return item;
			}

			currentObject = currentObject->next;
		}

		return nullptr;
	}
};
#pragma pack(pop)

template <typename T>
class CGGuidList {
public:
	CGGuidList(CGGuid* head, size_t size, size_t startIndex)
		: head(head), size(size), startIndex(startIndex) { }
	CGGuidList(CGGuid* head, size_t size)
		: head(head), size(size), startIndex(0) { }
	T* operator[](size_t index)
	{
		auto objectManager = *reinterpret_cast<CGObjectManager**>(BaseAddress() + Offsets::ObjectManager);
		return reinterpret_cast<T*>(objectManager->getByGuid(&head[startIndex + index]));
	}

	CGGuid* head;
	size_t size;
	size_t startIndex;
};

