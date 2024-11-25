#include "main.hpp"
#include "app.hpp"

void AppManagerTask(void *pvParameters)
{
  AppManager appManager;
  while (true)
  {
    appManager.application();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void app_main()
{
  xTaskCreate(AppManagerTask, "AppManagerTask", 4096, NULL, 1, NULL);
}