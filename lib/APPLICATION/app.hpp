#include <stdio.h>

class AppManager
{
public:
  AppManager();
  virtual ~AppManager();
  void application();
  static void confirmationHandler(const char *topic, const char *message);
  void initialize();
};