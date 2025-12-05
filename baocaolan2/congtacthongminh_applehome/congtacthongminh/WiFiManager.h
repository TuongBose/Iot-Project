#ifndef WiFiManager_h
#define WiFiManager_h

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <arduino_homekit_server.h>

extern "C" {
  #include "user_interface.h"
}

const char HTTP_HEADER[] PROGMEM          = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#00bcd4;color:#000000;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEADER_END[] PROGMEM      = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Kết nối WiFi</button></form><br/><form action=\"/i\" method=\"get\"><button>Thông tin thiết bị</button></form><br/><form action=\"/r\" method=\"post\"><button>Ðặt lại thiết bị</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='Tên Wifi'><br/><input id='p' name='p' length=64 type='password' placeholder='Mật Khẩu'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>Kết Nối</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Tìm Wifi</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Đã kết nối thành công.<br />Vui lòng thoát ra.<br />Gmail:lemanhtuong2109@gmail.com</div>";
const char HTTP_END[] PROGMEM             = "</div><br />Mọi thắc mắc liên hệ Lê Mạnh Tường</body></html>";

#ifndef WIFI_MANAGER_MAX_PARAMS
#define WIFI_MANAGER_MAX_PARAMS 10
#endif

class WiFiManagerParameter {
  public:
    /** 
        Create custom parameters that can be added to the WiFiManager setup web page
        @id is used for HTTP queries and must not contain spaces nor other special characters
    */
    WiFiManagerParameter(const char *custom);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    ~WiFiManagerParameter();

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManager;
};


class WiFiManager
{
  public:
    WiFiManager();
    ~WiFiManager();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

  // nếu bạn muốn luôn khởi động cổng cấu hình mà không cần cố gắng kết nối trước 
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // lấy tên AP của cổng cấu hình, vì vậy nó có thể được sử dụng trong lệnh gọi lại 
    String        getConfigPortalSSID();

    void          resetSettings();

   // đặt thời gian chờ trước khi vòng lặp máy chủ web kết thúc và thoát ngay cả khi chưa thiết lập.
     // hữu ích cho các thiết bị không thể kết nối tại một số điểm và bị mắc kẹt trong vòng lặp máy chủ web 
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    // đặt thời gian chờ để cố gắng kết nối, hữu ích nếu bạn nhận được nhiều kết nối không thành công 
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    // mặc định không hiển thị bất kỳ thứ gì dưới 1% chất lượng tín hiệu nếu được gọi 
    void          setMinimumSignalQuality(int quality = 1);
   // đặt cấu hình ip / gateway / subnet tùy chỉnh 
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    // đặt cấu hình cho một IP tĩnh 
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
   // được gọi khi chế độ AP và cổng cấu hình được khởi động 
    void          setAPCallback( void (*func)(WiFiManager*) );
    // được gọi khi cài đặt đã được thay đổi và kết nối thành công 
    void          setSaveConfigCallback( void (*func)(void) );
    // thêm thông số tùy chỉnh, trả về false khi không thành công 
    bool          addParameter(WiFiManagerParameter *p);
    // nếu điều này được đặt, nó sẽ thoát sau khi cấu hình, ngay cả khi kết nối không thành công. 
    void          setBreakAfterConfig(boolean shouldBreak);
   // nếu điều này được đặt, hãy thử thiết lập WPS khi bắt đầu (điều này sẽ làm trì hoãn cổng cấu hình trong tối đa 2 phút)
     //LÀM
     // nếu điều này được đặt, hãy tùy chỉnh kiểu 
    void          setCustomHeadElement(const char* element);
    // nếu điều này là đúng, hãy xóa các Điểm truy cập trùng lặp - mặc định đúng 
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEADER = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManager*) = NULL;
    void (*_savecallback)(void) = NULL;

    int                    _max_params;
    WiFiManagerParameter** _params;

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("KHÔNG TỪ PHƯƠNG PHÁP Chuỗi TRÊN Địa chỉ IP, bạn cần ESP8266 lõi 2.1.0 hoặc mới hơn để cấu hình IP Tùy chỉnh hoạt động. ");
      return false;
    }
};

#endif
