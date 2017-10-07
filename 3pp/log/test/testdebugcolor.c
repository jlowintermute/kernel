#include <log/log.h>

int
main(void)
{
  log_init_color(LOG_DEBUG);

  LOG(LOG_DEBUG, "This is the debug text: %s", "with a substring");
  LOG(LOG_INFO, "This is the info text: %s", "with a substring2");
  LOG(LOG_WARNING, "This is the warning text: %s", "with a substring3");
  LOG(LOG_ERROR, "This is the error text: %s", "with a substring4");
  LOG(LOG_FATAL, "This is the fatal text: %s", "with a substring5");
}
