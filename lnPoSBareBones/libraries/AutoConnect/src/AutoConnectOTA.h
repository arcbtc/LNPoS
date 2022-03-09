/**
 * Declaration of AutoConnectOTA class.
 * The AutoConnecOTA class is a class for web updating a Sketch binary
 * via OTA and implements with an AutoConnectAux page handler that
 * inherits from AutoConnectUploadHandler.
 * By overriding the _write function of AutoConnectUploadHandler to
 * write the executable binary using the Update class, it can update
 * the module firmware in synchronization with the upload of the sketch
 * binary file.
 * @file AutoConnectOTA.h
 * @author hieromon@gmail.com
 * @version  1.3.0
 * @date 2021-04-08
 * @copyright  MIT license.
 */

#ifndef _AUTOCONNECTOTA_H_
#define _AUTOCONNECTOTA_H_

#include <functional>
#include <memory>
#include "AutoConnect.h"
#include "AutoConnectTypes.h"
#include "AutoConnectUpload.h"
#include "AutoConnectFS.h"

class AutoConnectOTA : public AutoConnectUploadHandler {
public:
  // Type declaration of callback function to notify OTA status change
  typedef std::function<void(void)> StartExit_ft;
  typedef std::function<void(void)> EndExit_ft;
  typedef std::function<void(uint8_t)> ErrorExit_ft;
  typedef std::function<void(unsigned int, unsigned int)> ProgressExit_ft;

  // Updating process status
  typedef enum  {
    AC_OTA_IDLE,          /**< Update process has not started */
    AC_OTA_START,         /**< Update process has started */
    AC_OTA_PROGRESS,      /**< Update process in progress */
    AC_OTA_SUCCESS,       /**< A binary updater has uploaded fine */
    AC_OTA_RIP,           /**< Ready for module restart */
    AC_OTA_FAIL           /**< Failed to save binary updater by Update class */
  } AC_OTAStatus_t;

  // The treating destination of OTA transferred data
  typedef enum {
    OTA_DEST_FILE, /**< To be upload the file */
    OTA_DEST_FIRM  /**< To update the firmware */
  } AC_OTADest_t;

  AutoConnectOTA() : extraCaption(nullptr), _cbStart(NULL), _cbEnd(NULL), _cbError(NULL), _cbProgress(NULL), _dest(OTA_DEST_FIRM), _status(AC_OTA_IDLE), _tickerPort(-1), _tickerOn(LOW) {};
  ~AutoConnectOTA();
  void  attach(AutoConnect& portal);                        /**< Attach itself to AutoConnect */
  void  authentication(const AC_AUTH_t auth);               /**< Set certain page authentication */
  String  error(void) const { return _err; }                /**< Returns current error string */
  void  menu(const bool post) { _auxUpdate->menu(post); }   /**< Enabel or disable arranging a created AutoConnectOTA page in the menu. */
  void  reset(void) { _status = AC_OTA_IDLE; }              /**< Reset the status */
  AC_OTAStatus_t  status(void) const { return _status; }    /**< Return a current error status of the Update class */ 
  AC_OTADest_t  dest(void) const { return _dest; }          /**< Return a current uploading destination */
  void  setTicker(int8_t pin, uint8_t on) { _tickerPort = pin, _tickerOn = on; }  /**< Set ticker LED port */
  AutoConnectOTA& onStart(StartExit_ft fn);                 /**< Register a callback for OTA start */
  AutoConnectOTA& onEnd(EndExit_ft fn);                     /**< Register a callback for OTA end */
  AutoConnectOTA& onError(ErrorExit_ft fn);                 /**< Register a callback for OTA error */
  AutoConnectOTA& onProgress(ProgressExit_ft fn);           /**< Register a callback for OTA in progress */

  const char* extraCaption;                                 /**< Updating firmware extra caption */

protected:
  template <typename T, size_t N> constexpr size_t lengthOf(T (&)[N]) noexcept {
    return N;
  }
  void    _buildAux(AutoConnectAux* aux, const AutoConnectAux::ACPage_t* page, const size_t elementNum);
  bool    _open(const char* filename, const char* mode) override;
  size_t  _write(const uint8_t *buf, const size_t size) override;
  void    _close(const HTTPUploadStatus status) override;
  String  _updated(AutoConnectAux& result, PageArgument& args);

  std::unique_ptr<AutoConnectAux> _auxUpdate;   /**< An update operation page */
  std::unique_ptr<AutoConnectAux> _auxResult;   /**< An update result page */

  /**< A status change notification callback functions */
  StartExit_ft    _cbStart;
  EndExit_ft      _cbEnd;
  ErrorExit_ft    _cbError;
  ProgressExit_ft _cbProgress;  

 private:
  void  _setError(void);
  void  _setError(const char* err);

  AC_OTADest_t _dest;           /**< Destination of OTA transferred data */
  AC_OTAStatus_t  _status;      /**< Status for update progress */
  int8_t  _tickerPort;          /**< GPIO for flicker */
  uint8_t _tickerOn;            /**< A signal for flicker turn on */
  String  _binName;             /**< An updater file name */
  String  _err;                 /**< Occurred error stamp */
  size_t  _ulAmount;            /**< Cumulative amount uploaded */

  AutoConnectFS::FS*  _fs;      /**< Filesystem for the native file uploading */
  fs::File  _file;              /**< File handler for the native file uploading */

  static const AutoConnectAux::ACPage_t         _pageUpdate  PROGMEM;
  static const AutoConnectAux::ACElementProp_t  _elmUpdate[] PROGMEM;
  static const AutoConnectAux::ACPage_t         _pageResult  PROGMEM;
  static const AutoConnectAux::ACElementProp_t  _elmResult[] PROGMEM;
};

#endif // !_AUTOCONNECTOTA_H_
