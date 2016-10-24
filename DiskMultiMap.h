#pragma once
#ifndef DISKMULTIMAP_H_
#define DISKMULTIMAP_H_

#include <string>
#include <unordered_set>
#include <functional>
#include "MultiMapTuple.h"
#include "BinaryFile.h"

class DiskMultiMap
{
public:

	class Iterator
	{
	public:
		Iterator();
		// You may add additional constructors
		Iterator(BinaryFile::Offset currentPosToSearch, std::string fileName, std::string keyName);
		bool isValid() const;
		Iterator& operator++();
		MultiMapTuple operator*();

	private:
		// Your private member declarations will go here
		bool m_isValid;
		BinaryFile::Offset m_currPos;
		std::string m_fileToOpen;
		std::string m_currKey;

		struct subBucket
		{
			char m_key[121];
			char m_Val[121];
			char m_Cont[121];
			BinaryFile::Offset m_NextVal;
		};
	};

	DiskMultiMap();
	~DiskMultiMap();
	bool createNew(const std::string& filename, unsigned int numBuckets);
	bool openExisting(const std::string& filename);
	void close();
	bool insert(const std::string& key, const std::string& value, const std::string& context);
	Iterator search(const std::string& key);
	int erase(const std::string& key, const std::string& value, const std::string& context);

private:
	// Your private member declarations will go here
	string m_fileName;
	//std::unordered_set<MultiMapTuple> m_HashTable;
	BinaryFile m_Binary;
	unsigned int m_bucketsInFile;
	const int HEADER_SIZE = sizeof(HeaderStorage);
	const int BUCKETS_SIZE = sizeof(BucketsToPlace);			// sizes of the various structures
	const int SUB_BUCKET_SIZE = sizeof(subBucket);

	BinaryFile::Offset hashThisKey(const std::string & hashMe) const;	// function to hash the string to the proper bucket

	struct HeaderStorage						// header storage to be able to re-use nodes
	{
		int m_totalBucketsInFile;
		int m_bucketsDeleted;
		BinaryFile::Offset m_firstDeleted;
	};

	struct BucketsToPlace
	{
		BinaryFile::Offset m_first;			// "pointer" to the first subbucket
		bool m_used;						
	};

	struct subBucket
	{
		char m_key[121];
		char m_value[121];					// value that the key from the main bucket maps to
		char m_context[121];				// context that the key from the main bucket maps to
		BinaryFile::Offset m_next;		// this will point to the next node like a "linked" list
	};
};

#endif // DISKMULTIMAP_H_