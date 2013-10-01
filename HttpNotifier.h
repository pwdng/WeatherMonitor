#ifndef _HttpNotifier_h
#define _HttpNotifier_h

#include <string>
using namespace std;

class HttpNotifier {

  public:
    HttpNotifier(string);
    ~HttpNotifier();
    
    void sendValue(float value);

  private:
    string m_url;
  
};

#endif
