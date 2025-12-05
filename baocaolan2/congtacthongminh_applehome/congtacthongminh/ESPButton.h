#ifndef ESPBUTTON_H_
#define ESPBUTTON_H_

#include <Arduino.h>
#include <functional>
#include <Ticker.h>

#define ESPBUTTON_DEBUG(message, ...)  //printf_P(PSTR("[%7d] ESPButton: " message "\n"), millis(), ##__VA_ARGS__)

typedef struct _ESPButtonEntry {
  uint8_t id = -1;
  uint8_t pin = -1;
  uint8_t pin_down_digital = LOW; 

  bool stable_down = false;
  uint32_t stable_threshold = 40; // Nếu trạng thái nút không thay đổi trong một khoảng thời gian, nó được coi là ổn định
  bool is_stable = false;  
  bool raw_down = false; 
  uint32_t raw_changed_time = 0; 


  std::function<uint8_t(uint8_t pin)> ext_digitalRead = nullptr;
  //======
  bool longclicked = false; // Được sử dụng để đảm bảo rằng một lần nhấn chỉ có thể kích hoạt một lần nhấn và giữ
  bool down_handled = false; // Cho biết sự kiện báo chí xuống đã được xử lý hay chưa (ví dụ: một báo chí dài đã được kích hoạt)
  // xuống-> lên, xuống một lần nữa trong thời gian quy định là nhấp đúp, ngược lại là nhấp
  bool wait_doubleclick = false; // Đánh dấu có đợi sự kiện nhấp đúp hay không
  uint32_t down_time = 0; //ms
  uint32_t up_time = 0;

  uint32_t longclick_threshold = 15000;
  uint32_t doubleclick_threshold = 150; // Nhấn và thả lại trong khoảng thời gian này, được coi là nhấp đúp

  bool longclick_enable = true;
  bool doubleclick_enable = true;
  //======
  struct _ESPButtonEntry *next;
} ESPButtonEntry;

enum ESPButtonEvent {
  ESPBUTTONEVENT_NONE = 0,
  ESPBUTTONEVENT_SINGLECLICK,
  ESPBUTTONEVENT_DOUBLECLICK,
  ESPBUTTONEVENT_LONGCLICK
};
class ESPButtonClass;

static void _esp32_ticker_cb(ESPButtonClass *esp_button);

class ESPButtonClass {

  public:

    typedef std::function<void(uint8_t id, ESPButtonEvent event)> espbutton_callback;

    Ticker ticker;
    ESPButtonEntry *entries = nullptr;
    espbutton_callback callback;
    ESPButtonEvent notify_event = ESPBUTTONEVENT_NONE;
    uint8_t notify_id = 0;

    ESPButtonClass() {
    }
    ~ESPButtonClass() {
    }

    void begin() {
      ticker.detach();
#if defined(ESP8266)
      ticker.attach_ms(16, std::bind(&ESPButtonClass::tick, this));
#elif defined(ESP32)
      ticker.attach_ms(16, _esp32_ticker_cb, this);
#endif
    }

    ESPButtonEntry* add(uint8_t _id, uint8_t _pin, uint8_t _pin_down_digital,
                        bool _doubleclick_enable = false, bool _longclick_enable = true) {
      ESPButtonEntry *entry = new ESPButtonEntry();
      entry->id = _id;
      entry->pin = _pin;
      entry->pin_down_digital = _pin_down_digital;
      entry->doubleclick_enable = _doubleclick_enable;
      entry->longclick_enable = _longclick_enable;

      
      entry->next = entries;
      entries = entry;
      return entry;
    }

    void setCallback(espbutton_callback _callback) {
      callback = _callback;
    }

    PGM_P getButtonEventDescription(ESPButtonEvent e) {
      switch (e) {
        case ESPBUTTONEVENT_SINGLECLICK:
          return PSTR("SingleClick");
        case ESPBUTTONEVENT_DOUBLECLICK:
          return PSTR("DoubleClick");
        case ESPBUTTONEVENT_LONGCLICK:
          return PSTR("LongClick");
        default:
          return PSTR("<unknown event>");
      }
    }

    void tick() {
      ESPButtonEntry *entry = entries;
      while (entry) {
        tickEntry(entry);
        entry = entry->next;
      }
    }

    void loop() {
      if (callback && (notify_event != ESPBUTTONEVENT_NONE)) {
        callback(notify_id, notify_event);
        notify_id = 0;
        notify_event = ESPBUTTONEVENT_NONE;
      }
    }

  private:

    bool digitalReadEntryIsDown(ESPButtonEntry *entry) {
      if (entry->ext_digitalRead) {
        return entry->ext_digitalRead(entry->pin) == entry->pin_down_digital;
      }
      return digitalRead(entry->pin) == entry->pin_down_digital;
    }

    void tickEntry(ESPButtonEntry *entry) {
      const uint32_t t = millis();
      const bool down = digitalReadEntryIsDown(entry);
      if (down != entry->raw_down) {
        entry->raw_down = down;
        entry->is_stable = false;
        entry->raw_changed_time = t;
        ESPBUTTON_DEBUG("change (%s)", down ? PSTR("down") : PSTR("up"));
      } else { // down == raw_down
        // 在stable_threshold时间内一直没有变化，认为是stable
        if (!entry->is_stable) {
          if (t - entry->raw_changed_time > entry->stable_threshold) {
            ESPBUTTON_DEBUG("t: %d, raw: %d", t, entry->raw_changed_time); ESPBUTTON_DEBUG("stable (%s)", down ? PSTR("down") : PSTR("up"));
            entry->is_stable = true;
          }
        }
      }
      if (!entry->is_stable) {
        //ESPBUTTON_DEBUG("not stable");
        return;
      }
// Đoạn mã trên có thể phát hiện trạng thái ổn định trong hơn một khoảng thời gian nhất định và đợi nó ổn định trước khi xử lý
      if (entry->stable_down == down) {
        handleEntryUnchanged(entry);
        return;
      } else {
        entry->stable_down = down;
        handleEntryChanged(entry);
      }

    }

    void handleEntryChanged(ESPButtonEntry *entry) {
      const bool down = entry->stable_down;
      // Chỉ sự kiện click mới gọi lại trực tiếp khi nó bị sập? Đừng làm điều này vào lúc này. Nó giống như một công tắc vật lý. Khi bạn nhấn vào nó và không buông ra, nó sẽ luôn bật.
      // Logic như sau:
      // Nhấp: nhấn -> thả -> và không có lần nhấn thứ hai trong một khoảng thời gian
      // Nhấp đúp: nhấn -> thả -> và nó được kích hoạt khi nhấn lần thứ hai được thực hiện trong một khoảng thời gian
      // Nhấn lâu: được nhấn trong một khoảng thời gian và không được giải phóng
      if (down) { //down
        if (entry->wait_doubleclick && entry->doubleclick_enable) {
          // Lần thứ hai nó bị lỗi trong thời gian quy định, nó được coi là một cú nhấp đúp
          // Kiểm tra chuyên nghiệp, nói chung, khoảng thời gian giữa lần nhấp đúp của tôi lên-> lần xuống thứ hai là khoảng 80 ~ 100
          ESPBUTTON_DEBUG("doubleclick, wait %d", (millis() - entry->up_time));
          entry->down_handled = true;
          notifyEvent(entry, ESPBUTTONEVENT_DOUBLECLICK);
        } else {
          // Lần nhấn đầu tiên
          entry->down_handled = false;
        }
        entry->down_time = millis();
        entry->longclicked = false;
        entry->wait_doubleclick = false;
      } else { //up
        if (!entry->down_handled) {
          if (entry->doubleclick_enable) {
            // Trì hoãn vòng lặp và đợi lần nhấn thứ hai
            entry->up_time = millis();
            entry->wait_doubleclick = true;
          } else {
            entry->down_handled = true;
            notifyEvent(entry, ESPBUTTONEVENT_SINGLECLICK);
          }
        }
      }

    }

    void handleEntryUnchanged(ESPButtonEntry *entry) {
      bool down = entry->stable_down;
      if (down) { //down
        if (entry->longclick_enable) {
          if (!entry->longclicked && !entry->down_handled) {
            if (millis() - entry->down_time > entry->longclick_threshold) {
              entry->longclicked = true;
              entry->down_handled = true;
              notifyEvent(entry, ESPBUTTONEVENT_LONGCLICK);
            }
          }
        }
      } else { //up
        entry->longclicked = false;
        if (entry->wait_doubleclick && entry->doubleclick_enable) {
          if (millis() - entry->up_time > entry->doubleclick_threshold) {
            entry->wait_doubleclick = false;
            entry->down_handled = true;
            //key2DoClick();
            notifyEvent(entry, ESPBUTTONEVENT_SINGLECLICK);
          }
        }

      }
    }

    void notifyEvent(ESPButtonEntry *entry, ESPButtonEvent event) {
      ESPBUTTON_DEBUG("Button(%d): %s", entry->id, getButtonEventDescription(event));
      // Save the Event and notify it in loop
      if (notify_event != ESPBUTTONEVENT_NONE) {
        ESPBUTTON_DEBUG("Warnning! Previous Button Event is not handled in loop!");
      }
      notify_event = event;
      notify_id = entry->id;
      //    if (callback) {
      //      callback(entry->id, event);
      //    }
    }

};

ESPButtonClass ESPButton;

static void _esp32_ticker_cb(ESPButtonClass *esp_button) {
  esp_button->tick();
}

#endif /* ESPBUTTON_H_ */
