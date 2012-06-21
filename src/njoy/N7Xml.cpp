

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
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "njoy-pvraddon-agent/1.0");
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

    XMLResults xe;
    XMLNode xMainNode = XMLNode::parseString(strXML.c_str(), NULL, &xe);

    if(xe.error != 0)  {
      XBMC->Log(LOG_ERROR, "%s Unable to parse XML. Error: '%s' ", __FUNCTION__, XMLNode::getError(xe.error)); 
      return;
    }

      XBMC->Log(LOG_ERROR, "%s Unable to parse XML. Error: '%s' ", __FUNCTION__, XMLNode::getError(xe.error)); 

    XMLNode xNode = xMainNode.getChildNode("rss");
    xNode = xNode.getChildNode("channel");
    int n = xNode.nChildNode("item");

    XBMC->Log(LOG_INFO, "%s Number of elements: '%d'", __FUNCTION__, n);

    int iUniqueChannelId = 0;

    for(int i=0; i<n; i++)
    {
      XMLNode xTmp = xNode.getChildNode("item", i);
      CStdString strTmp;
      int iTmp;

      PVRChannel channel;
        
      /* unique ID */
      channel.iUniqueId = ++iUniqueChannelId;

      /* channel number */
      if (!GetInt(xTmp, "number", channel.iChannelNumber))
          channel.iChannelNumber = channel.iUniqueId;
        
      /* channel name */
      if (!GetString(xTmp, "title", strTmp))
        continue;
      channel.strChannelName = strTmp;
        
      /* icon path */
      XMLNode xMedia = xTmp.getChildNode("media:thumbnail");
      strTmp = xMedia.getAttribute("url");

      channel.strIconPath = strTmp;

      /* channel url */
      if (!GetString(xTmp, "guid", strTmp))
        channel.strStreamURL = "";
      else
        channel.strStreamURL = strTmp;
        
      m_channels.push_back(channel);
    
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


bool N7Xml::GetInt(XMLNode xRootNode, const char* strTag, int& iIntValue)
{
  XMLNode xNode = xRootNode.getChildNode(strTag );
  if (xNode.isEmpty())
     return false;
  iIntValue = atoi(xNode.getText());
  return true;
}

bool N7Xml::GetString(XMLNode xRootNode, const char* strTag, CStdString& strStringValue)
{
  XMLNode xNode = xRootNode.getChildNode(strTag );
  if (!xNode.isEmpty())
  {
    strStringValue = xNode.getText();
    return true;
  }
  strStringValue.Empty();
  return false;
}



