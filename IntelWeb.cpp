#include "IntelWeb.h"
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <cstring>
#include <cassert>
#include <unordered_set>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <queue>
#include <set>

using namespace std;

IntelWeb::IntelWeb()			
{
	m_maxItems = 0;
	m_filePrefix = "";				// initializes some member variables
	m_prevLevel = 0;
}

IntelWeb::~IntelWeb()			
{
	m_InitiatorEntry.close();			// closes all the binary files
	m_TargetEntry.close();
	m_prevCounter.close();
	m_filePrefix = "";
}

// must run in O(max items) and it currently follows that!
bool IntelWeb::createNew(const std::string & filePrefix, unsigned int maxDataItems)
{
	m_filePrefix = filePrefix;						// sets some member variables, like the file prefix and the max data items with a .75 load
	m_maxItems = maxDataItems/.75;

	bool e = m_InitiatorEntry.createNew(m_filePrefix + "initiator.dat", m_maxItems);		// creates a new binary file for each of the 3 diskmaps
	bool f = m_TargetEntry.createNew(m_filePrefix + "target.dat", m_maxItems);
	bool g = m_prevCounter.createNew(m_filePrefix + "prev.dat", m_maxItems);
	if (e == true && f == true && g == true)
		return true;
	else
		return false;
}

// must run in O (1) time
bool IntelWeb::openExisting(const std::string & filePrefix)
{
	m_filePrefix = filePrefix;

	bool e = m_InitiatorEntry.openExisting(m_filePrefix + "initiator.dat");			// tries to open exisitng diskmaps with the appropiate names (if they exist!)
	bool f = m_TargetEntry.openExisting(m_filePrefix + "target.dat");
	bool g = m_prevCounter.openExisting(m_filePrefix + "prev.dat");

	if (e == true && f == true && g == true)
		return true;
	else
		return false;
}

void IntelWeb::close()
{
	m_InitiatorEntry.close();
	m_TargetEntry.close();							// closes all the disk-multimaps 
	m_prevCounter.close();
	m_filePrefix = "";
}

// Big O(number of lines)
bool IntelWeb::ingest(const std::string & telemetryFile)
{
	ifstream inf(telemetryFile);
	if (!inf)															// if it can't open the telemetry data, returns false
	{
		cerr << "cannot open this telemetry file" << endl;
		return false;
	}
	string line;
	m_InitiatorEntry.openExisting(m_filePrefix + "initiator.dat");
	m_TargetEntry.openExisting(m_filePrefix + "target.dat");

	while (getline(inf, line))											// otherwise it opens the data, and takes in each line
	{
		istringstream iss(line);
		string firstStr;
		string secondStr;
		string thirdStr;
		if (!(iss >> firstStr >> secondStr >> thirdStr))
		{
			cerr << "Ignoring badly-formatted input line: " << line << endl;		// if the line is badly formatted, skips over it
			continue;
		}
		//char dummy;
		//if (iss >> dummy)
		//	cerr << "Ignoring extra data in line: " << line << endl;			// to ignore anything that comes after the last string

		m_InitiatorEntry.insert(secondStr, thirdStr, firstStr);					// inserts the 3 strings into two maps that get the "to" and the "from" relationship that programs and websites can have
		m_TargetEntry.insert(thirdStr, secondStr, firstStr);

		updatePrevalence(secondStr);											// updates the prevalences of each key and value so that it can be used in crawl below.
		updatePrevalence(thirdStr);
	}
	return true;
}

// Max of O(t*log(t))
unsigned int IntelWeb::crawl(const std::vector<std::string>& indicators, unsigned int minPrevalenceToBeGood, std::vector<std::string>& badEntitiesFound, std::vector<InteractionTuple>& badInteractions)
{
	badEntitiesFound.clear();
	badInteractions.clear();					
	m_prevLevel = minPrevalenceToBeGood;
	m_InitiatorEntry.openExisting(m_filePrefix + "initiator.dat");
	m_TargetEntry.openExisting(m_filePrefix + "target.dat");
	set<string> possibleBadIndicators;
	set<InteractionTuple> possibleBadInteractions;								// various STL data structures that only take a unique item so that the I don't go into an infinite loop
	queue<string> indicatorsToSearch;											// and anything I insert into the data structures is automatically sorted for me
	unordered_set<string> hashOfEntities;

	for (int i = 0; i < indicators.size(); i++)
	{
		indicatorsToSearch.push(indicators[i]);									// add the bad indicators to the queue and to the hashtable of indicators
		hashOfEntities.insert(indicators[i]);
	}

	while(!indicatorsToSearch.empty())
	{
		string toSearch = indicatorsToSearch.front();
		indicatorsToSearch.pop();

		DiskMultiMap::Iterator initiatorSearch = m_InitiatorEntry.search(toSearch);		// look at the front item of the queue and search for it

		while (initiatorSearch.isValid())
		{
			MultiMapTuple a = *initiatorSearch;
			auto it = hashOfEntities.find(toSearch);								
			
			if (it != hashOfEntities.end())							// if its in the hash table, and it was an indicator, then we add it to the possibleBadEntities since it was discovered
			{
				possibleBadIndicators.insert(toSearch);
			}
			int z = getPrevalence(a.value);
		//	cerr << " the prev was : " << z << endl;			// the interaction was tainted, so it should be added to the possible bad interactions
			possibleBadInteractions.insert(InteractionTuple(a.key, a.value, a.context));	// we add the interaction regardless since that value has been "Tainted" by the bad indicator
			if (z < m_prevLevel)
			{
				//cerr << "searching through the following initiator " << toSearch << endl;
			//	possibleBadEntities.insert(a.value);
				//if (possibleBadEntities.find(a.value) == possibleBadEntities.end())			
				//{
				//	possibleBadEntities.insert(a.value);
				//	indicatorsToSearch.push(a.value);
				//}
				if (hashOfEntities.find(a.value) == hashOfEntities.end())			// if it is not in the possible set of bad identities, then it adds it to the queue to search		
																					// otherwise it was not the first item, so we should not add the queue of items to search for.
				{
					hashOfEntities.insert(a.value);
					possibleBadIndicators.insert(a.value);
					indicatorsToSearch.push(a.value);
				}
			}
			++initiatorSearch;
		}
		DiskMultiMap::Iterator targetSearch = m_TargetEntry.search(toSearch);		

		while (targetSearch.isValid())
		{
			MultiMapTuple a = *targetSearch;
			auto it = hashOfEntities.find(toSearch);							// repeat the above process, but looking through the second DiskMultiMap
			if (it != hashOfEntities.end())
			{
				possibleBadIndicators.insert(toSearch);
			}
			int z = getPrevalence(a.value);
		//	cerr << "the prev level was " << z << endl;
			possibleBadInteractions.insert(InteractionTuple(a.value, a.key, a.context));		// add the interaction regardless since the value was tainted
			if (z < m_prevLevel)								// only if the value is under the prev level then it should be added to the queue of indicators to be searched in
			{
				//cerr << "searching through the following target " << toSearch << endl;																	// should still insert (and if it was already inserted, its a set, so will not try to insert again
				/*if (possibleBadEntities.find(a.value) == possibleBadEntities.end())
				{
					possibleBadEntities.insert(a.value);
					indicatorsToSearch.push(a.value);
				}*/
				if (hashOfEntities.find(a.value) == hashOfEntities.end())			// if it is not in the possible set of bad identities, then it adds it to the queue to search									// otherwise it was not the first item, so we should not add the queue of items to search for.
				{
					hashOfEntities.insert(a.value);
					possibleBadIndicators.insert(a.value);
					indicatorsToSearch.push(a.value);
				}
			}
			++targetSearch;
		}
	} 		

	for (auto it = possibleBadIndicators.begin(); it != possibleBadIndicators.end(); it++)			// sort through the bad indicators and the bad interactions below so that our final vectors are 
																									// lexicographically sorted
	{
		badEntitiesFound.push_back((*it));
	}

	for (auto it = possibleBadInteractions.begin(); it != possibleBadInteractions.end(); it++)
	{
		badInteractions.push_back((*it));
	}
	// rules for malicious entries:
		// 0) If it was passed into indicators above:
		// 1) if the downloaded a file that is less than the prevValue, then downloaded file is bad
		// 2) if the downloaded file is bad, and the website that it comes from is below prev Level, it is also bad
		// 3) if first file is bad, and the file it makes is less than prevValue, then it is also bad
		// 4) if the second file is already bad, and the file it came from is less than prevValue, then it also bad
		// 5) if the program is bad, and the website it links to is less than the prevValue, then it is also bad
		// 6) if the website that a program leads to is bad, and the program/file that connected to it is less than prevValue, then it is also bad

	return badEntitiesFound.size();
}

// Runs in O (M) times where M is the possible number of entities that match the entity string
bool IntelWeb::purge(const std::string & entity)
{
	bool deletedAtLeastOne = false;
	string keyToFind = entity;
	m_InitiatorEntry.openExisting(m_filePrefix + "initiator.dat");
	m_TargetEntry.openExisting(m_filePrefix + "target.dat");

	for (auto it = m_InitiatorEntry.search(keyToFind); it.isValid(); it = m_InitiatorEntry.search(keyToFind))			// finds all instances of the key in the initiator DiskMultimap
	{
		deletedAtLeastOne = true;
		MultiMapTuple a = *it;
		int dec = m_InitiatorEntry.erase(a.key, a.value, a.context);					// erase the key and value and context from the initiator entry
		//cerr << "purged at least items: " << dec << endl;	
		decrementPrevalence(a.value, dec);												// decrement the prevalence of the value				
		m_TargetEntry.erase(a.value, a.key, a.context);									// erase the opposite order, value ->key->context from the target map
	}
	for (auto it = m_TargetEntry.search(keyToFind); it.isValid(); it = m_TargetEntry.search(keyToFind))
	{
		deletedAtLeastOne = true;
		MultiMapTuple a = *it;
		m_InitiatorEntry.erase(a.value, a.key, a.context);								// erase from the initiator in the opposite way as the target, (val, key, context)
		int dec = m_TargetEntry.erase(a.key, a.value, a.context);						// erase from the target in the correct order, (key, val, context)
		decrementPrevalence(a.value, dec);												// decrement the prevalence accordingly
		//cerr << "purged at least items: " << dec << endl;
	}
	if (deletedAtLeastOne)
		setPrevToZero(keyToFind);				// set the prevalence of the entity to zero (since all should have been deleted by now
	return deletedAtLeastOne;
}

// runs in O (1) time
void IntelWeb::updatePrevalence(string key)
{
	int newPrev = (getPrevalence(key));
	m_prevCounter.erase(key, to_string(newPrev), "");				// increases the prevalence by 1 for that particular key (using to_string to convert an int to a string)
	m_prevCounter.insert(key, to_string(newPrev+1), "");
}

// runs in O(1) time
void IntelWeb::decrementPrevalence(string key, int a)
{
	int newPrev = getPrevalence(key);
	m_prevCounter.erase(key, to_string(newPrev), "");				// decrement the prevalence by the passed in amount
	m_prevCounter.insert(key, to_string(newPrev-a), "");
}

// runs in O(1) time
int IntelWeb::getPrevalence(string key)
{
	DiskMultiMap::Iterator it = m_prevCounter.search(key);
	if (it.isValid())
	{
		MultiMapTuple a = *it;									// get the prevalence of the key at that point (and use stoi to convert the string to an int)
		return stoi(a.value);
	}
	return 0;
}

// runs in O(1) time
void IntelWeb::setPrevToZero(string key)
{
	m_prevCounter.erase(key, to_string(getPrevalence(key)), "");			// set the prevalence of the key to 0
	m_prevCounter.insert(key, to_string(0), "");
}


bool operator< (const InteractionTuple &a, const InteractionTuple &b)			// operator < overload for Interaction Tuple to store in lexicographic order in the vector
{
	if (a.context < b.context)
		return true;
	if (a.context == b.context)							// first orders by context (the machine numbers), then by the key, and then by the value (ie from, and then to)
	{
		if (a.from < b.from)
			return true;
		if (a.from == b.from)
		{
			if (a.to < b.to)
				return true;
		}
	}
	return false;
}