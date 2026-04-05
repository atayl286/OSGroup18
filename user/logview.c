#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static const char *event_name(int event_code) {
  switch (event_code) {
    case LOG_EVENT_NORMAL:
      return "NORMAL";
    case LOG_EVENT_SPIKE:
      return "SPIKE";
    case LOG_EVENT_SHUTDOWN:
      return "SHUTDOWN";
    default:
      return "UNKNOWN";
  }
}

int main(void) {
  struct turbine_log log;
  if (get_log(&log) < 0) {
    printf("logview: get_log failed\n");
    exit(1);
  }

  if (!log.log_count) {
    printf("No log entries.\n");
    exit(0);
  }

  printf(
    "%s \t%s \t%s \t%s\n",
    "#",
    "Timestamp",
    "RPM",
    "Event"
  );

  // if the buffer is full, then the oldest log is at the log_head
  int start = (log.log_count == LOG_SIZE) ? log.log_head : 0;

  int avg_rpm = 0;
  int min_rpm = log.log_buf[start].rpm;
  int max_rpm = log.log_buf[start].rpm;

  // go through each entry of the log buffer
  for (int i = 0; i < log.log_count; i++) {
    int idx = (start + i) % LOG_SIZE;
    struct log_entry *entry = &log.log_buf[idx];
    // print the log entry
    printf(
      "%d \t%u \t\t%d \t%s\n",
      i + 1,
      entry->timestamp,
      entry->rpm,
      event_name(entry->event)
    );
    // avg_rpm
    avg_rpm += entry->rpm;
    // max_rpm
    if (entry->rpm >= max_rpm) {
      max_rpm = entry->rpm;
    }
    // min_rpm
    if (entry->rpm <= min_rpm)
      min_rpm = entry->rpm;
  }

  // finish avg rpm calculation
  avg_rpm /= log.log_count;

  printf("--SUMMARY--\n");
  printf("Total Log Entries: %d\n", log.log_count);
  printf("Max RPM: %d\n", max_rpm);
  printf("Min RPM: %d\n", min_rpm);
  printf("Avg RPM: %d\n", avg_rpm);

  exit(0);
}
