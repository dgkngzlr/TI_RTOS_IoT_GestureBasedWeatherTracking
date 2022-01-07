# TI_RTOS_IoT_GestureBasedWeatherTracking

In this project an IoT solution was developed for weather tracking. To implement it following equipments are used :

* 1-) Texas Instruments TM4C_1294NCPDT Development Board (TM4C_1294XL)
* 2-) PAJ7620 Gesture Recognition Sensor
* 3-) Ethernet Cable

To implement the project TI's Real Time Operating System was used.All synchronization in the multi-threading structure was provided by semaphores.Using I2C interface, sensor data is taken. If the gesture flag is "RIGHT" then humidity and weather condition is taken from the API. In case of gesture flag is "LEFT" then temperature and weather condition is taken from the API. The weather data is taken from openweather.org API using by HTTP requests and UTC time info is taken from NTP server. The UTC time which is taken from the NTP server, was converted to UTC+03 time zone to use the application in local time zone. In addition that, each generated data package is printed to a TCP server on the computer. In this way, all information can be viewed on remote server.
