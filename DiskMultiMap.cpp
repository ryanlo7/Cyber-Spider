#include "DiskMultiMap.h"
#include "MultiMapTuple.h"
#include <cstring>
#include <functional>

DiskMultiMap::DiskMultiMap()
{

}
void DiskMultiMap::close()
{
	bf.close();
}
DiskMultiMap::~DiskMultiMap()
{
	close();
}
void DiskMultiMap::resetConstants()
{
	int counter = 0;
	NUM_BUCKETS = counter;
	counter += sizeof(NUM_BUCKETS);
	NEXT_OPEN = counter;
	counter += sizeof(NEXT_OPEN);
	DELETED_HEAD = counter;
	counter += sizeof(DELETED_HEAD);
	EMPTY_SIZE = counter;
	counter += sizeof(EMPTY_SIZE);
	FIRST_NODE = counter;
}
int DiskMultiMap::hashFunc(const std::string s)
{
	std::hash<std::string> str_hash;
	unsigned int h = str_hash(s);
	int numBuckets;
	bf.read(numBuckets, NUM_BUCKETS);
	return 20 + 4 * ((h % numBuckets) - 1);
}

bool DiskMultiMap::createNew(const std::string& filename, unsigned int numBuckets)
{
	close();

	bf.createNew(filename);
	this->filename = filename;

	NUM_BUCKETS = 0;
	bf.write(numBuckets, NUM_BUCKETS); // int numBuckets written at 0
									   //current offset = 4;
	int counter = sizeof(NUM_BUCKETS); //counter at 4
	NEXT_OPEN = counter;
	bf.write(NULL_OFFSET, counter);
	counter += sizeof(NULL_OFFSET); //counter at 8
	if (!bf.write(NULL_OFFSET, counter))			//offset for address of deleted data space
		return false;
	DELETED_HEAD = counter;
	counter += sizeof(NULL_OFFSET); //counter at 12
	EMPTY_SIZE = counter;
	if (!bf.write(0, counter))			//int num deleted nodes (size)
		return false;
	counter += sizeof(NULL_OFFSET); //counter at 16
	int temp = counter + sizeof(NULL_OFFSET);
	FIRST_NODE = counter;
	if (!bf.write(temp, counter))			//Offset for the first node of map
		return false;
	BinaryFile::Offset nextEmpty;
	if (!bf.read(nextEmpty, FIRST_NODE))
		return false;
	for (int i = 0; i < numBuckets; i++)
	{
		bf.write(NULL_OFFSET, nextEmpty);
		nextEmpty += sizeof(NULL_OFFSET);
	}
	bf.write(nextEmpty, NEXT_OPEN);


	return true;
}

bool DiskMultiMap::openExisting(const std::string& filename)
{
	close();
	if (bf.openExisting(filename) == false)
		return false;
	resetConstants();
	return true;
}

bool DiskMultiMap::insert(const std::string& key, const std::string& value,
	const std::string& context)
{
	if (key.size() > 120 || value.size() > 120 || context.size() > 120)
		return false;
	DiskNode d;
	strcpy(d.key, key.c_str());
	strcpy(d.value, value.c_str());
	strcpy(d.context, context.c_str());
	/*cout << "New Node Properties: " << endl;
	cout << "\t" << d.key << endl;
	cout << "\t" << d.value << endl;
	cout << "\t" << d.context << endl;*/


	BinaryFile::Offset deleteHead;
	bf.read(deleteHead, DELETED_HEAD);
	int id = hashFunc(key);			//get the bucket id
	if (deleteHead == NULL_OFFSET)
	{
		BinaryFile::Offset head, nextEmpty;
		bf.read(head, id);
		bf.read(nextEmpty, NEXT_OPEN);

		d.next = head;
		bf.write(d, nextEmpty);	//insert the new node at the next empty space
		bf.write(nextEmpty, id); //update the head ptr in the hashmap
		nextEmpty = nextEmpty + sizeof(d);
		bf.write(nextEmpty, NEXT_OPEN); //update next open space
		return true;
	}
	else
	{

		BinaryFile::Offset head, topDeleteOffset; //head = head of hashmap bucket, 
												  //TDO = offset to head of deleetd node linked list
		bf.read(head, id);				//get the head offset of hashmap bucket
		d.next = head;		//set new node "d" next offset = head

		bf.read(topDeleteOffset, DELETED_HEAD);//get the offset to top head of deleted nodes
		bf.write(topDeleteOffset, id);			//UPDATES head offset of the given bucket

		DiskNode dn;
		bf.read(dn, topDeleteOffset);		//get the node at the top of head of deleted nodes
		bf.write(dn.next, DELETED_HEAD);	//update head of deleted nodes to point to next in the list

		bf.write(d, topDeleteOffset);		//put new node into this space

	}

	return true;

}

int DiskMultiMap::erase(const std::string& key, const std::string& value,
	const std::string& context)
{
	int numRemoved = 0;
	BinaryFile::Offset prev = hashFunc(key), cur;
	int id = prev;
	//int id = hashFunc(key);			//get the bucket id
	char c[121], k[121], v[121];
	strcpy(k, key.c_str());
	strcpy(v, value.c_str());
	strcpy(c, context.c_str());

	bf.read(cur, prev);
	if (cur == NULL_OFFSET) return 0;
	DiskNode d;
	while (cur != NULL_OFFSET)
	{
		bf.read(d, cur);
		if (strcmp(k, d.key) == 0 && strcmp(v, d.value) == 0 && strcmp(c, d.context) == 0)
		{
			BinaryFile::Offset curNext = d.next;
			if (id == prev) //for if prev is the head
			{
				bf.write(d.next, prev);
				numRemoved++;
			}
			else {
				DiskNode d1;
				bf.read(d1, prev);
				d1.next = d.next;
				bf.write(d1, prev);
				numRemoved++;
			}
			bf.read(d.next, DELETED_HEAD);
			bf.write(d, cur);
			bf.write(cur, DELETED_HEAD);
			cur = curNext;
		}
		else {
			prev = cur;
			cur = d.next;
		}
	}
	return numRemoved;
}

DiskMultiMap::Iterator DiskMultiMap::search(const std::string& key)
{
	char k[121];
	strcpy(k, key.c_str());

	int id = hashFunc(key);
	BinaryFile::Offset cur;
	bf.read(cur, id);
	if (cur == NULL_OFFSET) {
		DiskMultiMap::Iterator i;
		return i;
	}
	DiskNode d;
	while (NULL_OFFSET != cur)
	{
		bf.read(d, cur);
		if (strcmp(k, d.key) == 0)
		{
			DiskMultiMap::Iterator it(&bf, cur, d.next);
			return it;
		}
		cur = d.next;
	}

	DiskMultiMap::Iterator it;
	return it;
}
DiskMultiMap::Iterator::Iterator()
{
	valid = false;
	bf = nullptr;
	cur = next = -1;
}
DiskMultiMap::Iterator::Iterator(BinaryFile* b, BinaryFile::Offset cur, BinaryFile::Offset next)
{
	bf = b;
	this->cur = cur;
	this->next = next;
	valid = true;
}
bool DiskMultiMap::Iterator::isValid() const
{
	return valid;
}
MultiMapTuple DiskMultiMap::Iterator::operator*()
{
	if (!isValid())
	{
		MultiMapTuple m;
		return m;
	}
	DiskNode d;
	bf->read(d, cur);
	MultiMapTuple map;
	map.key = d.key;
	map.value = d.value;
	map.context = d.context;
	return map;
}
DiskMultiMap::Iterator & DiskMultiMap::Iterator::operator++()
{
	if (isValid())
	{
		cur = next;
		if (next == NULL_OFFSET)
			valid = false;
		else {
			DiskNode d;
			bf->read(d, cur);
			next = d.next;
		}
	}
	return *this;
}
