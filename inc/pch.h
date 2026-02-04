#include <stdio.h>
#include <string>
#include <map>
#include <unordered_map>
#include <tchar.h>
#include <vector>
#include <mutex>
#include <thread>
#include <regex>
#include <atomic>
#include <chrono>
#include <iostream>
#include <fstream>
#include <sstream>
#include <bitset> //std::bitset
#include <deque>
#include <set>
#include <numeric>
#include <algorithm>
#include <format>
#include <memory>
#include <random>
#include <iomanip> 
#include <filesystem>
#include <variant>
#include <functional>
#include <condition_variable>
#include <cmath>
#include <queue>


#include <winsock2.h> //note: winsock2.h must be included before windows.h to avoid errors
#include <ws2tcpip.h>
#include <winhttp.h>
#include <wininet.h>
#include <bcrypt.h>
#include <Wincrypt.h>
#include <iphlpapi.h>
#include <intrin.h>
#include <windowsx.h>  // for GET_X_LPARAM, GET_Y_LPARAM
#include <ShlObj.h>
#include <shellapi.h>  // For ShellExecuteEx and SHELLEXECUTEINFO
#include <Shlwapi.h>
#include <accctrl.h> //EXPLICIT_ACCESS and other security attributes
#include <aclapi.h> //to enable SetEntriesInAclA function
#include <lmcons.h> // For UNLEN
#include <propidl.h>
#include <comdef.h> //for the _com_error class
#include <strsafe.h> //to enable StringCchPrintf
#include <Wtsapi32.h>
#include <Sddl.h>
#include <devguid.h>  // For GUID_DEVCLASS_xxx
#include <cfgmgr32.h> // For CM_xxx functions
#include <SetupAPI.h>
//#include <secext.h> // For GetUserNameEx and name formats
#include <atlconv.h>  // for OLE2T
#include <atlbase.h>  // for CComPtr
#include <atlcom.h>
#include <Dshow.h> // For CLSID_VideoInputDeviceCategory
#include <tlhelp32.h>  // For CreateToolhelp32Snapshot, Process32First, Process32Next
#include <ShellScalingApi.h> //needed for GetDpiForMonitor call
#include <wincodec.h>
#include <gdiplus.h>
#include <http.h>  // For HTTP Server API
#include <dshow.h>
#include <dwmapi.h> //needed for round UI window courners
#include <dvdmedia.h>  // This includes VIDEOINFOHEADER2 definition
#include <wrl.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
//#ifdef OPEN_SSL_ENABLED_APP
#include <openssl/err.h>
//#endif //OPEN_SSL_ENABLED_APP

#ifndef _WIN32
#include <X11/Xlib.h>
#endif

#pragma warning(disable : 4864)
#include <opencv2/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/objdetect/face.hpp>
#pragma warning(default : 4864)

