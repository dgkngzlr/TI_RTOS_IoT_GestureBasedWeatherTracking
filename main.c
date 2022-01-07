/*STL*/
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/Error.h>
#include <xdc/runtime/System.h>

/* TI-RTOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/hal/Seconds.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/drivers/GPIO.h>
#include <ti/net/http/httpcli.h>
#include <ti/net/sntp/sntp.h>
#include <ti/drivers/I2C.h>
/* Example/Board Header file */
#include "Board.h"
#include "paj7620.h"
#include <sys/socket.h>

/*Configs*/
#define HOSTNAME          "api.openweathermap.org"
#define REQUEST_URI       "/data/2.5/weather?q=eskisehir&appid=1b532b43e65ca38c5b76ebc70e293274"
#define USER_AGENT        "HTTPCli (ARM; TI-RTOS)"
#define NTP_HOSTNAME     "pool.ntp.org"
#define NTP_PORT         "123"
#define NTP_SERVERS      3
#define NTP_SERVERS_SIZE (NTP_SERVERS * sizeof(struct sockaddr_in))
#define HTTPTASKSTACKSIZE 4096*2
#define SOCKETTEST_IP     "192.168.1.5"
#define SOCKETPORT        5050

unsigned char ntpServers[NTP_SERVERS_SIZE];
static Semaphore_Handle semHandle = NULL;

extern Semaphore_Handle semRight;
extern Semaphore_Handle semLeft;
extern Semaphore_Handle semHttp;
extern Semaphore_Handle semTcp;

char   tempstr[256];
int isRight = 0;
char data[512];
char desc[64];
char hum[64];
char temp[64];
char msg[256];

/*
 *  ======== printError ========
 */
/*Parse key value from given data or string*/
void parseString(char* str_, char* key_,char end, char val[]){

    char str[512];
    char key[64];
    int len_key = strlen(key_);

    strcpy(str,str_);
    strcpy(key,key_);

    char *ret = strstr(str, key);

    char *e;
    int index1, index2;

    e = strchr(ret, ':');
    index1 = (int)(e - ret);

    e = strchr(ret, end);
    index2 = (int)(e - ret);

    strncpy(val,&ret[index1+1],index2-index1-1);
    val[index2] = '\0';


}

/*Get UTC+03 Time Zone*/
void getLocalTime(char date[], char localTime[])
{
    char * token = strtok(date, " ");
    char * day = token;


    token = strtok(NULL, " ");
    char * month = token;


    token = strtok(NULL, " ");
    char * dayNum = token;


    token = strtok(NULL, " ");
    char * time = token;
    char * temptime = time;


    token = strtok(NULL, " ");
    char * year = token;


    char * hour = strtok(temptime, ":");
    char * min = strtok(NULL, ":");
    char * sec = strtok(NULL, ":");

    int hourVal = atoi(hour);
    hourVal += 3;

    char newHour[2];

    sprintf(newHour, "%d", hourVal);

    sprintf(localTime, "%s %s  %s %s:%s:%s %s",day, month,dayNum, newHour, min, sec, year);


}
void printError(char *errString, int code)
{
    System_printf("Error! code = %d, desc = %s\n", code, errString);
    BIOS_exit(code);
}

/*Send data to TCP Server*/
void sendData2Server(char *serverIP, int serverPort, char *data, int size)
{
    int sockfd;
    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd == -1) {
        System_printf("Socket not created");
        BIOS_exit(-1);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));  /* clear serverAddr structure */
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);     /* convert port # to network order */
    inet_pton(AF_INET, serverIP, &(serverAddr.sin_addr));

    int connStat = connect(sockfd, (struct sockaddr *)&serverAddr, /* connecting….*/
                  sizeof(serverAddr));
    if(connStat < 0) {
        System_printf("Error while connecting to server\n");
        if (sockfd > 0)
            close(sockfd);
        BIOS_exit(-1);
    }

    int numSend = send(sockfd, data, size, 0);       /* send data to the server*/
    if(numSend < 0) {
        System_printf("Error while sending data to server\n");
        if (sockfd > 0) close(sockfd);
        BIOS_exit(-1);
    }

    if (sockfd > 0) close(sockfd);
}

/*Parse Humidity data*/
Void getHumTask(UArg arg0, UArg arg1){

    while(1) {
        Semaphore_pend(semRight, BIOS_WAIT_FOREVER);
        char* key = "humidity";
        parseString(data, key, '}',hum);
        key = "description";
        parseString(data, key, ',',desc);
        System_printf("Humidity : %s - Description : %s\n", hum, desc);
        System_flush();

        Semaphore_post(semTcp);


    }

}
/*Parse Temperature data*/
Void getTempTask(UArg arg0, UArg arg1){
    float temp_value, celcius_value;
    while(1) {
        Semaphore_pend(semLeft, BIOS_WAIT_FOREVER);
        char* key = "temp";
        parseString(data, key, ',',temp);

        temp_value = atof(temp);
        celcius_value = temp_value - 272.15; // Kelvin to C degree
        sprintf(temp, "%.1f", celcius_value);

        key = "description";
        parseString(data, key, ',',desc);
        System_printf("Temp (C) : %s - Description : %s\n", temp, desc);
        System_flush();

        Semaphore_post(semTcp);

    }

}

/*Create data package which is sent to Server*/
Void socketTask(UArg arg0, UArg arg1)
{
    while(1) {
        Semaphore_pend(semTcp, BIOS_WAIT_FOREVER);

        time_t ts = time(NULL);
        char localTime[128];
        char date[128];

        sprintf(date, "%s", ctime(&ts));

        System_flush();
        getLocalTime(date, localTime);


        if (isRight == 1){

            sprintf(msg, "%s >> Humidity : %s | Condition : %s\n", localTime, hum, desc);
        }
        else {
            sprintf(msg, "%s >> Temperature (C) : %s | Condition : %s\n", localTime, temp, desc);
        }
        GPIO_write(Board_LED0, 1); // turn on the LED

        // connect to SocketTest program on the system with given IP/port
        // send hello message whihc has a length of 5.
        //
        sendData2Server(SOCKETTEST_IP, SOCKETPORT, msg, strlen(msg));



        // wait for 5 seconds (5000 ms)
        //
        Task_sleep(1000);
    }
}

/*
 *  ======== timeUpdateHook ========
 *  Called after NTP time sync
 */
void timeUpdateHook(void *p)
{
    Semaphore_post(semHandle);
}

/*
 *  ======== startNTP ========
 */
/*Get UTC time from API*/
void startNTP(void)
{
    int ret;
    int currPos;

    struct sockaddr_in ntpAddr;
    struct addrinfo hints;
    struct addrinfo *addrs;
    struct addrinfo *currAddr;
    Semaphore_Params semParams;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;

    ret = getaddrinfo(NTP_HOSTNAME, NTP_PORT, NULL, &addrs);
    if (ret != 0) {
        printError("startNTP: NTP host cannot be resolved!", ret);
    }

    currPos = 0;

    for (currAddr = addrs; currAddr != NULL; currAddr = currAddr->ai_next) {
        if (currPos < NTP_SERVERS_SIZE) {
            ntpAddr = *(struct sockaddr_in *)(currAddr->ai_addr);
            memcpy(ntpServers + currPos, &ntpAddr, sizeof(struct sockaddr_in));
            currPos += sizeof(struct sockaddr_in);
        }
        else {
            break;
        }
    }

    freeaddrinfo(addrs);

    ret = SNTP_start(Seconds_get, Seconds_set, timeUpdateHook,
            (struct sockaddr *)&ntpServers, NTP_SERVERS, 0);
    if (ret == 0) {
        printError("startNTP: SNTP cannot be started!", -1);
    }

    Semaphore_Params_init(&semParams);
    semParams.mode = Semaphore_Mode_BINARY;
    semHandle = Semaphore_create(0, &semParams, NULL);
    if (semHandle == NULL) {
        printError("startNTP: Cannot create semaphore!", -1);
    }

    SNTP_forceTimeSync();
    Semaphore_pend(semHandle, BIOS_WAIT_FOREVER);

}

/*
 *  ======== httpsTask ========
 *  Makes an HTTP GET request
 */
/*Get weather data from API*/
Void httpTask(UArg arg0, UArg arg1)
{
    bool moreFlag = false;
    //char data[64];
    int ret;
    int len;
    struct sockaddr_in addr;

    HTTPCli_Struct cli;
    HTTPCli_Field fields[3] = {
        { HTTPStd_FIELD_NAME_HOST, HOSTNAME },
        { HTTPStd_FIELD_NAME_USER_AGENT, USER_AGENT },
        { NULL, NULL }
    };

    startNTP();
    Task_sleep(1000);

    while(1) {


        System_printf("httpTask Waiting !\n");
        System_flush();

        Semaphore_pend(semHttp, BIOS_WAIT_FOREVER);

        System_printf("httpTask Started !\n");
        System_flush();

        HTTPCli_construct(&cli);

        HTTPCli_setRequestFields(&cli, fields);

        ret = HTTPCli_initSockAddr((struct sockaddr *)&addr, HOSTNAME, 0);
        if (ret < 0) {
            printError("httpTask: address resolution failed", ret);
        }

        ret = HTTPCli_connect(&cli, (struct sockaddr *)&addr, 0, NULL);
        if (ret < 0) {
            printError("httpTask: connect failed", ret);
        }

        ret = HTTPCli_sendRequest(&cli, HTTPStd_GET, REQUEST_URI, false);
        if (ret < 0) {
            printError("httpTask: send failed", ret);
        }

        ret = HTTPCli_getResponseStatus(&cli);
        if (ret != HTTPStd_OK) {
            printError("httpTask: cannot get status", ret);
        }

        System_printf("HTTP Response Status Code: %d\n", ret);

        ret = HTTPCli_getResponseField(&cli, data, sizeof(data), &moreFlag);
        if (ret != HTTPCli_FIELD_ID_END) {
            printError("httpTask: response field processing failed", ret);
        }

        len = 0;
        do {
            ret = HTTPCli_readResponseBody(&cli, data, sizeof(data), &moreFlag);
            if (ret < 0) {
                printError("httpTask: response body processing failed", ret);
            }

            if (isRight == 1){
                Semaphore_post(semRight);
            }

            else {
                Semaphore_post(semLeft);
            }

            len += ret;
        } while (moreFlag);


        HTTPCli_disconnect(&cli);
        HTTPCli_destruct(&cli);
    }

}

/*Endless while loop to take gesture*/
Void paj7620Task(UArg arg0, UArg arg1){

    I2C_Handle      i2c;
    I2C_Params      i2cParams;
    I2C_Transaction i2cTransaction;
    uint8_t suc;
    uint8_t data;

    /* Create I2C for usage */
    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c = I2C_open(Board_I2C0, &i2cParams);
    if (i2c == NULL) {
        System_abort("Error Initializing I2C\n");
    }
    else {
        System_printf("I2C Initialized!\n");
    }
    System_flush();
    Task_sleep(100);
    ////////////////////////////////////////////////
    suc = paj7620Init(i2c, i2cTransaction);
    while(1 && suc){
        data = paj7620ReadReg(i2c, i2cTransaction, PAJ7620_ADDR_GES_PS_DET_FLAG_0, 1, &data);
        switch (data) {
           // GET WEATHER COND. AND HUMIDITY
            case GES_RIGHT_FLAG:
                System_printf("RIGHT\n");
                System_flush();
                ////////////////////////
                isRight = 1;
                Semaphore_post(semHttp);
                ////////////////////////
                break;

            // GET WEATHER COND. AND TEMP
            case GES_LEFT_FLAG:
                System_printf("LEFT\n");
                System_flush();
                ////////////////////////
                isRight = 0;
                Semaphore_post(semHttp);
                ////////////////////////
                break;
        }
        Task_sleep(100);
    }

}

/*
 *  ======== netIPAddrHook ========
 *  This function is called when IP Addr is added/deleted
 */
void netIPAddrHook(unsigned int IPAddr, unsigned int IfIdx, unsigned int fAdd)
{
    static Task_Handle taskHandle, taskHandle1;
    Task_Params taskParams;
    Error_Block eb;

    /* Create a HTTP task when the IP address is added */
    if (fAdd && !taskHandle && !taskHandle1) {
        Error_init(&eb);

        Task_Params_init(&taskParams);
        taskParams.stackSize = HTTPTASKSTACKSIZE;
        taskParams.priority = 1;
        taskHandle = Task_create((Task_FuncPtr)httpTask, &taskParams, &eb);

        Task_Params_init(&taskParams);
        taskParams.stackSize = HTTPTASKSTACKSIZE;
        taskParams.priority = 1;
        taskHandle1 = Task_create((Task_FuncPtr)socketTask, &taskParams, &eb);

        if (taskHandle == NULL || taskHandle1 == NULL) {
            printError("netIPAddrHook: Failed to create HTTP Task\n", -1);
        }
    }
}

/*
 *  ======== main ========
 */
int main(void)
{
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    Board_initEMAC();
    Board_initI2C();

    /* Turn on user LED */
    GPIO_write(Board_LED0, Board_LED_ON);

    System_printf("==========STARTING=========\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
