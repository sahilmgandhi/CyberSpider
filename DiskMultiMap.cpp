#define _CRT_SECURE_NO_WARNINGS

#include "DiskMultiMap.h"

#include <iostream>
#include <cstring>
#include <cassert>
#include <unordered_set>
#include <algorithm>

using namespace std;

class Iterator;

DiskMultiMap::Iterator::Iterator()
{
	m_isValid = false;									// sets all member variables to default state, and by default the iterator is invalid
	m_currPos = 0;
	m_fileToOpen = "";
	m_currKey = "";
}

// runs in O(1) times
DiskMultiMap::Iterator::Iterator(BinaryFile::Offset currentPosToSearch, string fileName, string keyName)
{
	m_isValid = true;
	m_currPos = currentPosToSearch;				// if the following is passed in, then the iterator is valid, and the appropiate member variables are updated
	m_fileToOpen = fileName;
	m_currKey = keyName;
}

// runs in O(1) times
bool DiskMultiMap::Iterator::isValid() const
{	
	return m_isValid;							// returns the valid state of the iterator
}

DiskMultiMap::Iterator& DiskMultiMap::Iterator::operator++()
{
	if (!isValid())
	{
		return *(this);				// if the iterator is not valid, then it does not do anything
	}

	BinaryFile m_itBinary;
	subBucket s;
	m_itBinary.openExisting(m_fileToOpen);				
	m_itBinary.read(s, m_currPos);
	m_currPos = s.m_NextVal;
	m_itBinary.read(s, m_currPos);						// reads in the current sub-bucket, and then reads in the next (cause we are doing ++ prefix operator)
	while (m_currPos > 0)			
	{
		m_itBinary.read(s, m_currPos);
		if (strcmp(s.m_key, m_currKey.c_str()) == 0)		// in case collision happens, ONLY if the key is correct, will it return the iterator
		{
			m_itBinary.close();
			return (*this);
		}
		m_currPos = s.m_NextVal;							// otherwise we will try to increment it again
	}
	m_itBinary.close();
	m_isValid = false;									// if the next pos is bad, then it sets it to an invalid state
	return *(this);
}

// runs in O(1) time
MultiMapTuple DiskMultiMap::Iterator::operator*()
{	
	BinaryFile m_itBinary;
	m_itBinary.openExisting(m_fileToOpen);
	subBucket s;
	m_itBinary.read(s, m_currPos);				// reads the context and value off the iterator
	MultiMapTuple a;
	a.key = s.m_key;
	a.value = s.m_Val;
	a.context = s.m_Cont;						// converts them into string to put into the Maptuple object
	m_itBinary.close();
	return a;							
}

// runs in O(1) time
DiskMultiMap::DiskMultiMap()
{
	m_fileName = "";							// initialize member variables of the disk multimap
	m_bucketsInFile = 0;
}

// runs in O(1) time
DiskMultiMap::~DiskMultiMap()
{
	m_Binary.close();							// destructs the diskmultimap by closing the binary file
}

// Running in O (B) times where B is the number of buckets
bool DiskMultiMap::createNew(const std::string & filename, unsigned int numBuckets)
{
	m_Binary.close();							// closing any earlier data file
	bool b = m_Binary.createNew(filename);		// creating the new data file
	m_fileName = filename;				
	m_bucketsInFile = numBuckets;
	if (b)
	{
		HeaderStorage a;
		a.m_totalBucketsInFile = m_bucketsInFile;
		a.m_bucketsDeleted = 0;							// at the beginning, there are no buckets being used.
		a.m_firstDeleted = -1;
		m_Binary.write(a, 0);

		for (unsigned int i = 0; i < m_bucketsInFile; i++)
		{
			BucketsToPlace b;
			b.m_used = false;							// buckets that are initialized are not being used it
			b.m_first = -1;								// buckets that are initialized dont have any sub buckets yet
			m_Binary.write(b, m_Binary.fileLength());			// puts each bucket at the end of the list
		}
		return true;
	}
	return false;												// if this function fails to make a new hash table for some reason
}

// runs in O (1) times
bool DiskMultiMap::openExisting(const std::string & filename)
{	
	m_Binary.close();

	bool alreadyExists = m_Binary.openExisting(filename);					// close old file and then see if the file already exists
	if (alreadyExists)
	{
		m_fileName = filename;	
		HeaderStorage h;
		m_Binary.read(h, 0);
		m_bucketsInFile = h.m_totalBucketsInFile;
		return true;											// TODO: add more to change the m_bucketSize appropiately 
	}
	else
		return false;
}

// runs in O (1) times
void DiskMultiMap::close()
{
	m_Binary.close();									// closes the binary file 
	m_fileName = "";
}

// Currently runs in O (1) times
bool DiskMultiMap::insert(const std::string & key, const std::string & value, const std::string & context)
{
	if (key.size() > 120 || value.size() > 120 || context.size() > 120)			// too large strings cannot be put and returns false
		return false;

	m_Binary.openExisting(m_fileName);
	BinaryFile::Offset whereToPut = hashThisKey(key);
	BucketsToPlace b;
	m_Binary.read(b, whereToPut);
	if (b.m_used == false)												// if the bucket hasn't been used before, then it sets the first 
	{																	// "pointer" to the first sub-bucket to be inserted
		subBucket s;
		strcpy(s.m_key, key.c_str());
		strcpy(s.m_value, value.c_str());
		strcpy(s.m_context, context.c_str());

		HeaderStorage h;
		m_Binary.read(h, 0);
		if (h.m_bucketsDeleted > 0)										// however, we first check if there are any deleted buckets to be re-used
		{
			h.m_bucketsDeleted--;
			subBucket firstDel;
			m_Binary.read(firstDel, h.m_firstDeleted);

			b.m_first = h.m_firstDeleted;
			h.m_firstDeleted = firstDel.m_next;							// we appropiately put the sub bucket where the first bucket to be re-used is, 
			s.m_next = -1;												// and then increment the first sub-bucket to be re-used for further inserts		
			m_Binary.write(s, b.m_first);
			b.m_used = true;
			m_Binary.write(b, whereToPut);

			m_Binary.write(h, 0);
		}
		else
		{
			b.m_first = m_Binary.fileLength();							// otherwise, if there are no buckets to re-use, we just put it at the end of the file
			s.m_next = -1;
			m_Binary.write(s, m_Binary.fileLength());
			b.m_used = true;
			m_Binary.write(b, whereToPut);
		}
		return true;
	}
	else
	{
		subBucket s;													// if there is already something hashed to the bucket then:
		strcpy(s.m_key, key.c_str());
		strcpy(s.m_value, value.c_str());
		strcpy(s.m_context, context.c_str());
		HeaderStorage h;
		m_Binary.read(h, 0);
		if (h.m_bucketsDeleted > 0)										// once again, using the same logic as above, we either insert the sub-bucket at the first
		{																// position to be re-used, OR we add it to the end of the file
			s.m_next = b.m_first;
			h.m_bucketsDeleted--;
			subBucket firstDel;
			m_Binary.read(firstDel, h.m_firstDeleted);
			b.m_first = h.m_firstDeleted;
			h.m_firstDeleted = firstDel.m_next;
			m_Binary.write(s, b.m_first);
			m_Binary.write(b, whereToPut);

			m_Binary.write(h, 0);
		}
		else
		{
			s.m_next = b.m_first;
			b.m_first = m_Binary.fileLength();							// added to the end of the file if there are no spots to eb re-used
			m_Binary.write(s, m_Binary.fileLength());
			m_Binary.write(b, whereToPut);
		}	
		return true;													// return true since something was inserted in the file
	}
	return false;		// returns false if it cannot do any of the above at all
}

// runs in O(1) time
DiskMultiMap::Iterator DiskMultiMap::search(const std::string & key)
{
	m_Binary.openExisting(m_fileName);
	BinaryFile::Offset whereToFind = hashThisKey(key);
	BucketsToPlace b;
	m_Binary.read(b, whereToFind);

	if ( b.m_used == false)						// if the bucket has not already been used to store a node, then it is a bad iterator
	{
		DiskMultiMap::Iterator a;
		return a;
	}
	else
	{
		DiskMultiMap::Iterator a(b.m_first, m_fileName, key);
		if ((*a).key == key)					// otherwise, the bucket is being used, BUT we still need to find the first isntance of the key
			return a;							// incase the hashed bucket contains collision
		++a;
		return a;
	}
}

// must run in O(N/b) to find, and then O(K) to delete if the key has K total possible items
int DiskMultiMap::erase(const std::string & key, const std::string & value, const std::string & context)
{
	int numDeleted = 0;
	DiskMultiMap::Iterator a = search(key);
	if (a.isValid())												// if can find the key using search, then continue ahead because one such sub-bucket exists
	{
		m_Binary.openExisting(m_fileName);
		BinaryFile::Offset posOfKey = hashThisKey(key);
		BucketsToPlace b;
		m_Binary.read(b, posOfKey);									// read in the bucket, and then read in the first Sub-bucket it points to
		subBucket currAssociation;
		m_Binary.read(currAssociation, b.m_first);					// the first sub-bucket is stored as currAssociation
		BinaryFile::Offset currSubBucket = b.m_first;
		BinaryFile::Offset nextSubBucket = b.m_first;
		HeaderStorage h;
		m_Binary.read(h, 0);

		if (strcmp(currAssociation.m_key, key.c_str()) == 0 && strcmp(currAssociation.m_value, value.c_str()) == 0 && strcmp(currAssociation.m_context, context.c_str())== 0) // if the first node needs to be removed
		{
			b.m_first = currAssociation.m_next;						// first now points to the 2nd node
			currSubBucket = currAssociation.m_next;					// currbucket is the position of the second node
			if (h.m_bucketsDeleted == 0)							
			{
				h.m_bucketsDeleted++;								// if there are no sub-buckets to re-use, then add the current position as a sub-bucket to re-use when inserting later
				h.m_firstDeleted = nextSubBucket;
				currAssociation.m_next = -1;
				strcpy(currAssociation.m_key, "");
				strcpy(currAssociation.m_value, "");
				strcpy(currAssociation.m_context, "");
				m_Binary.write(currAssociation, nextSubBucket);
				m_Binary.write(h, 0);
			}
			else if (h.m_bucketsDeleted > 0)						// else if there are alread buckets to re-use, link the deleted bucket to the 
			{														// value stored in the header storage, and then make the header storage's first offset equal to the current bucket
				h.m_bucketsDeleted++;
				currAssociation.m_next = h.m_firstDeleted;
				h.m_firstDeleted = nextSubBucket;
				strcpy(currAssociation.m_key, "");
				strcpy(currAssociation.m_value, "");
				strcpy(currAssociation.m_context, "");
				m_Binary.write(currAssociation, nextSubBucket);
				m_Binary.write(h, 0);
			}
			numDeleted++;
			if (currSubBucket > 0)									// if there is a 2nd node, then changes the currnt Association
			{
				m_Binary.write(b, posOfKey);
				nextSubBucket = currSubBucket;						// the next subbucket is the 3rd now. (or -1 if it doesnt exist)
			}
			else
			{
				b.m_used = false;
				b.m_first = -1;
				m_Binary.write(b, posOfKey);
				return numDeleted;
			}
		}
		else
		{
			nextSubBucket = currAssociation.m_next;					// otherwise we appropiately just move the next subbucket to the 2nd node (which could be -1)
		}

		while (nextSubBucket > 0)									// if more than 1 sub bucket exists, then we need to check if the next sub-bucket has the key, context, and value to be deleted
		{
			subBucket nextAssociation;
			m_Binary.read(nextAssociation, nextSubBucket);
			if ((strcmp(nextAssociation.m_key, key.c_str()) == 0) && (strcmp(nextAssociation.m_value, value.c_str()) == 0) && (strcmp(nextAssociation.m_context, context.c_str()) == 0))
			{
				if (b.m_first == nextSubBucket)									// if the bucket's first is the node to be deleted, then we change the bucket's next appropiately 	
				{
					b.m_first = nextAssociation.m_next;
					m_Binary.write(b, posOfKey);
				}
				subBucket aCpy = currAssociation;
				aCpy.m_next = nextAssociation.m_next;
				m_Binary.write(aCpy, currSubBucket);							// update the current sub-bucket with the next good sub-bucket and re-write it to the file
				BinaryFile::Offset n = nextSubBucket;
				nextSubBucket = nextAssociation.m_next;
				if (h.m_bucketsDeleted == 0)									// if there are no buckets to be re-used, then this deleted bucket is the first to be re-used
				{
					h.m_bucketsDeleted++;
					h.m_firstDeleted = n;
					currAssociation.m_next = -1;
					strcpy(currAssociation.m_key, "");
					strcpy(currAssociation.m_value, "");
					strcpy(currAssociation.m_context, "");
					m_Binary.write(currAssociation, n);
					m_Binary.write(h, 0);
				}
				else if (h.m_bucketsDeleted > 0)								// otherweise, like before, we add to the "linked list" of re-usable buckets
				{
					h.m_bucketsDeleted++;
					currAssociation.m_next = h.m_firstDeleted;
					h.m_firstDeleted = n;
					strcpy(currAssociation.m_key, "");
					strcpy(currAssociation.m_value, "");
					strcpy(currAssociation.m_context, "");
					m_Binary.write(currAssociation, n);
					m_Binary.write(h, 0);
				}
				numDeleted++;
			}
			else
			{	
				currSubBucket = currAssociation.m_next;							// otherwise,move the curr and the next associations forward to continue looking for items to delete
				currAssociation = nextAssociation;
				nextSubBucket = currAssociation.m_next;
			}
		}
		return numDeleted;
	}
	else
	{
		return 0;
	}
}

BinaryFile::Offset DiskMultiMap::hashThisKey(const std::string & hashMe) const
{
	std::hash<string> stringHasher;
	unsigned int hashVal = stringHasher(hashMe);													// uses the C++ functional hash method to hash strings to appropiate buckets
	unsigned int returnVal = HEADER_SIZE + (hashVal % m_bucketsInFile)*BUCKETS_SIZE;
	return returnVal;
}