#pragma once

#include "../platform/util/StdString.h"
#include "../tinyxml/tinyxml.h"
#include "client.h"
#include <vector>

struct WebResponse {
  char *response;
  int iSize;
};


struct PVRChannel
{
  int                     iUniqueId;
  int                     iChannelNumber;
  std::string             strChannelName;
  std::string             strIconPath;
  std::string             strStreamURL;
  
  PVRChannel() 
  { iUniqueId      = 0;
    iChannelNumber = 0;
    strChannelName = "";
    strIconPath    = "";
    strStreamURL   = "";    
  }
};

class N7Xml
{
public:
  N7Xml(void);
  ~N7Xml(void);
  int getChannelsAmount(void);
  PVR_ERROR requestChannelList(PVR_HANDLE handle, bool bRadio);
  PVR_ERROR requestEPGForChannel(PVR_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd);
  PVR_ERROR getSignal(PVR_SIGNAL_STATUS &qualityinfo);
  void list_channels(void);
private:
  CStdString GetHttpXML(CStdString& url);
  static int WebResponseCallback(void *contents, int iLength, int iSize, void *memPtr);
  std::vector<PVRChannel> m_channels;
  bool m_connected;
  static bool GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue);
  static bool GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringValue);
  static void Decode(CStdString& strURLData);
};

