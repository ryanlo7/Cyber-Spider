#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_
#define _CRT_SECURE_NO_WARNINGS

#include <string>
#include "BinaryFile.h"
#include <vector>
#include "MultiMapTuple.h"

const BinaryFile::Offset NULL_OFFSET = -1;

class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		//You may add additional constructors
		Iterator(BinaryFile* b, BinaryFile::Offset cur, BinaryFile::Offset next);
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();

	private:
		// Your private member declarations will go here
		bool valid;
		BinaryFile* bf;
		BinaryFile::Offset next, cur;
		/*struct DiskNode
		{
		char key[121];
		char value[121];
		char context[121];
		BinaryFile::Offset next;
		};*/
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);

	//delete this:
	// void dumpHashTable() {
		// /*for (BinaryFile::Offset i = 0; i < 40; i += 4)
		// {
		// BinaryFile::Offset j;
		// bf.read(j, i);
		// cout << j << endl;
		// }

		// */
		// BinaryFile::Offset i;
		// int numBuckets;
		// bf.read(numBuckets, NUM_BUCKETS);
		// DiskNode d;
		// for (i = 20; i < 20 + numBuckets * 4; i += 4)
		// {
			// BinaryFile::Offset j;
			// bf.read(j, i);
			// cout << "Bucket :" << (i - 20) / 4 << endl;

			// while (j != -1)
			// {
				// cout << "\t";
				// bf.read(d, j);
				// cout << d.key << " " << d.value << " " << d.context << endl;
				// j = d.next;
			// }
		// }
	// }
	// void dumpDeleted() {
		// BinaryFile::Offset head;
		// bf.read(head, DELETED_HEAD);
		// DiskNode d;
		// cout << endl << "Deleted Nodes:" << endl;

		// if (head == -1)
			// cout << "Deleted Linked list is empty" << endl;
		// else {
			// while (head != -1)
			// {
				// bf.read(d, head);
				// cout << "\t" << d.key << " " << d.value << " " << d.context << endl;
				// head = d.next;
			// }
		// }
	// }
private:
	// Your private member declarations will go here
	BinaryFile bf;
	std::string filename;
	int hashFunc(const std::string s);
	BinaryFile::Offset NUM_BUCKETS = 0, NEXT_OPEN = 0, DELETED_HEAD = 0,
		EMPTY_SIZE = 0, FIRST_NODE = 0;
	void resetConstants();
	struct DiskNode
	{
		char key[121];
		char value[121];
		char context[121];
		BinaryFile::Offset next;
	};
};

#endif // DISKMULTIMAP_H_
