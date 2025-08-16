// Local

// Internal Library
#include <logger.h>

// External Library
#include <ctime>

// Falls du String aus Arduino meinst:
#include <WString.h> // oder <string> für std::string

namespace DateTimeUtils {

    bool parseISODate(const char* iso, struct tm& tmOut);
    time_t timegm_portable(struct tm *tm);
    int getDayOfWeek(const String& iso);
    int getHour(const String& iso);
    int getMinute(const String& iso);
    int getDayOffsetFromWeekStart(const String& iso, const struct tm& weekStart);
    struct tm getWeekStart(const struct tm& someDate);
    struct tm getTodayAsWeekStart();

}
