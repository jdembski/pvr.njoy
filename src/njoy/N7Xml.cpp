

#include "N7Xml.h"
#include <curl/curl.h>
#include "client.h"

using namespace ADDON;



N7Xml::N7Xml(void)
{
  list_channels();
  bool m_connected = false;
}

N7Xml::~N7Xml(void)
{
  m_channels.clear();
}

int N7Xml::WebResponseCallback(void *contents, int iLength, int iSize, void *memPtr)
{
  int iRealSize = iSize * iLength;
  struct WebResponse *resp = (struct WebResponse*) memPtr;

  resp->response = (char*) realloc(resp->response, resp->iSize + iRealSize + 1);

  if (resp->response == NULL)
  {
    XBMC->Log(LOG_ERROR, "%s Could not allocate memeory!", __FUNCTION__);
    return 0;
  }

  memcpy(&(resp->response[resp->iSize]), contents, iRealSize);
  resp->iSize += iRealSize;
  resp->response[resp->iSize] = 0;

  return iRealSize;
}

CStdString N7Xml::GetHttpXML(CStdString& url)
{
  CURL* curl_handle;

  XBMC->Log(LOG_INFO, "%s Open URL: '%s'", __FUNCTION__, url.c_str());

  struct WebResponse response;

  response.response = (char*) malloc(1);
  response.iSize = 0;

  // retrieve the webpage and store it in memory
  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();
  curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, &WebResponseCallback);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&response);
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "vuplus-pvraddon-agent/1.0");
  curl_easy_perform(curl_handle);

  if (response.iSize == 0)
  {
    XBMC->Log(LOG_INFO, "%s Could not open webAPI", __FUNCTION__);
    return "";
  }

  CStdString strTmp;
  strTmp.Format("%s", response.response);

  XBMC->Log(LOG_INFO, "%s Got result. Length: %u", __FUNCTION__, strTmp.length());

  free(response.response);
  curl_easy_cleanup(curl_handle);

  return strTmp;
}



int N7Xml::getChannelsAmount()
{ 
	return m_channels.size();
}

void N7Xml::list_channels()
{
  CStdString strUrl;
  strUrl.Format("http://%s:%i/n7channel_nt.xml", g_strHostname.c_str(), g_iPort);
  CStdString strXML;

  strXML = GetHttpXML(strUrl);
  
  if(strXML.length() == 0)
  {
    XBMC->Log(LOG_DEBUG, "N7Xml - Could not open connection to N7 backend.");
  }
  else
  {
    m_connected = true;
    XBMC->Log(LOG_DEBUG, "N7Xml - Connected to N7 backend.");    
    TiXmlDocument xml;
    xml.Parse(strXML.c_str());
    TiXmlElement* rootXmlNode = xml.RootElement();
    TiXmlElement* channelsNode = rootXmlNode->FirstChildElement("channel");
    if (channelsNode)
    {
      int iUniqueChannelId = 0;
      TiXmlNode *pChannelNode = NULL;
      while ((pChannelNode = channelsNode->IterateChildren(pChannelNode)) != NULL)
      {  
        CStdString strTmp;
        PVRChannel channel;
        
        /* unique ID */
        channel.iUniqueId = ++iUniqueChannelId;
        
        /* channel number */
        if (!GetInt(pChannelNode, "number", channel.iChannelNumber))
          channel.iChannelNumber = channel.iUniqueId;
        
        /* channel name */
        if (!GetString(pChannelNode, "title", strTmp))
          continue;
        channel.strChannelName = strTmp;
        
        /* icon path */
        const TiXmlElement* pElement = pChannelNode->FirstChildElement("media:thumbnail");
        channel.strIconPath = pElement->Attribute("url");
        
        /* channel url */
        if (!GetString(pChannelNode, "guid", strTmp))
          channel.strStreamURL = "";
        else
          channel.strStreamURL = strTmp;
        
        m_channels.push_back(channel);
      }
    }  
  }
}


PVR_ERROR N7Xml::requestChannelList(PVR_HANDLE handle, bool bRadio)
{
  if (m_connected)
  {  
    std::vector<PVRChannel>::const_iterator item;
    PVR_CHANNEL tag;
    for( item = m_channels.begin(); item != m_channels.end(); ++item)
    {
      const PVRChannel& ch = *item; 
      memset(&tag, 0 , sizeof(tag));
      tag.iUniqueId       = ch.iUniqueId;
      tag.iChannelNumber  = ch.iChannelNumber;
      tag.strChannelName  = ch.strChannelName.c_str();
      tag.bIsRadio        = false;
      tag.strInputFormat  = "";
      tag.strStreamURL    = ch.strStreamURL.c_str();
      tag.strIconPath     = ch.strIconPath.c_str();
      tag.bIsHidden       = false;
      XBMC->Log(LOG_DEBUG, "N7Xml - Loaded channel - %s.", tag.strChannelName);
      PVR->TransferChannelEntry(handle, &tag);   
    }
  }
  else
  {
    XBMC->Log(LOG_DEBUG, "N7Xml - no channels loaded");
  }

	return PVR_ERROR_NO_ERROR;
}

PVR_ERROR N7Xml::requestEPGForChannel(PVR_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
{
	return PVR_ERROR_NO_ERROR;
}

PVR_ERROR N7Xml::getSignal(PVR_SIGNAL_STATUS &qualityinfo)
{
	return PVR_ERROR_NO_ERROR;
}


bool N7Xml::GetInt(const TiXmlNode* pRootNode, const char* strTag, int& iIntValue)
{
  const TiXmlNode* pNode = pRootNode->FirstChild(strTag );
  if (!pNode || !pNode->FirstChild()) return false;
  iIntValue = atoi(pNode->FirstChild()->Value());
  return true;
}

bool N7Xml::GetString(const TiXmlNode* pRootNode, const char* strTag, CStdString& strStringValue)
{
  const TiXmlElement* pElement = pRootNode->FirstChildElement(strTag );
  if (!pElement) return false;
  const char* encoded = pElement->Attribute("urlencoded");
  const TiXmlNode* pNode = pElement->FirstChild();
  if (pNode != NULL)
  {
    strStringValue = pNode->Value();
    if (encoded && strcasecmp(encoded,"yes") == 0)
      Decode(strStringValue);
    return true;
  }
  strStringValue.Empty();
  return false;
}

void N7Xml::Decode(CStdString& strURLData)
{
  CStdString strResult;

  /* result will always be less than source */
  strResult.reserve( strURLData.length() );

  for (unsigned int i = 0; i < strURLData.size(); ++i)
  {
    int kar = (unsigned char)strURLData[i];
    if (kar == '+') strResult += ' ';
    else if (kar == '%')
    {
      if (i < strURLData.size() - 2)
      {
        CStdString strTmp;
        strTmp.assign(strURLData.substr(i + 1, 2));
        int dec_num=-1;
        sscanf(strTmp,"%x",(unsigned int *)&dec_num);
        if (dec_num<0 || dec_num>255)
          strResult += kar;
        else
        {
          strResult += (char)dec_num;
          i += 2;
        }
      }
      else
        strResult += kar;
    }
    else strResult += kar;
  }
  strURLData = strResult;
}



