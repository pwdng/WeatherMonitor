#include "HttpNotifier.h"
#include <curl/curl.h>
#include <cstring>

HttpNotifier::HttpNotifier(string url): m_url(url) {
  curl_global_init(CURL_GLOBAL_ALL);
}

HttpNotifier::~HttpNotifier() {
  curl_global_cleanup();
}

void HttpNotifier::sendValue(float value) {
  CURL *curl;
  CURLcode res;

  /* get a curl handle */
  curl = curl_easy_init();
  if (curl) {
    char * cstr = new char [m_url.length()+1];
    strcpy (cstr, m_url.c_str());
    curl_easy_setopt(curl, CURLOPT_URL, cstr);

    /* Set the POST data
       Format expected by web server is "reading=x.xxxx" */
    char post_data[20];
    sprintf(post_data, "reading=%f", value);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post_data);

    /* Send request */
    res = curl_easy_perform(curl);

    /* Check for errors */
    if(res != CURLE_OK)
      fprintf(stderr, "curl_easy_perform() failed: %s\n",
              curl_easy_strerror(res));

    /* always cleanup */
    curl_easy_cleanup(curl);
  }
}
