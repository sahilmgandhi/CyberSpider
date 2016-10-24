#pragma once
#ifndef INTELWEB_H_
#define INTELWEB_H_

#include "InteractionTuple.h"
#include "DiskMultiMap.h"
#include <string>
#include <vector>

class IntelWeb
{
public:
	IntelWeb();
	~IntelWeb();
	bool createNew(const std::string& filePrefix, unsigned int maxDataItems);
	bool openExisting(const std::string& filePrefix);
	void close();
	bool ingest(const std::string& telemetryFile);
	unsigned int crawl(const std::vector<std::string>& indicators,
		unsigned int minPrevalenceToBeGood,
		std::vector<std::string>& badEntitiesFound,
		std::vector<InteractionTuple>& interactions
		);
	bool purge(const std::string& entity);

private:
	// Your private member declarations will go here
	unsigned int m_maxItems;
	unsigned int m_prevLevel;
	std::string m_filePrefix;
	// make some type of data structure to store the prevalence mode of everything when ingesting
	DiskMultiMap m_InitiatorEntry;
	DiskMultiMap m_TargetEntry;
	DiskMultiMap m_prevCounter;

	void updatePrevalence(std::string key);
	void decrementPrevalence(std::string key, int a);
	int getPrevalence(std::string key);
	void setPrevToZero(std::string key);
	//std::vector<std::string> 
};

#endif // INTELWEB_H_
