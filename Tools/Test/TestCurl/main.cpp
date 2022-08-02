#include "mainwindow.h"

#include <QApplication>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#ifndef WIN32
#  include <unistd.h>
#endif
#include <curl/curl.h>

#include <pthread.h>

static const char *urls[] =
{
    "https://www.microsoft.com",
    "https://opensource.org",
    "https://www.google.com",
    "https://www.yahoo.com",
    "https://www.ibm.com",
    "https://www.mysql.com",
    "https://www.oracle.com",
    "https://www.ripe.net",
    "https://www.iana.org",
    "https://www.amazon.com",
    "https://www.netcraft.com",
    "https://www.heise.de",
    "https://www.chip.de",
    "https://www.ca.com",
    "https://www.cnet.com",
    "https://www.mozilla.org",
    "https://www.cnn.com",
    "https://www.wikipedia.org",
    "https://www.dell.com",
    "https://www.hp.com",
    "https://www.cert.org",
    "https://www.mit.edu",
    "https://www.nist.gov",
    "https://www.ebay.com",
    "https://www.playstation.com",
    "https://www.uefa.com",
    "https://www.ieee.org",
    "https://www.apple.com",
    "https://www.symantec.com",
    "https://www.zdnet.com",
    "https://www.fujitsu.com/global/",
    "https://www.supermicro.com",
    "https://www.hotmail.com",
    "https://www.ietf.org",
    "https://www.bbc.co.uk",
    "https://news.google.com",
    "https://www.foxnews.com",
    "https://www.msn.com",
    "https://www.wired.com",
    "https://www.sky.com",
    "https://www.usatoday.com",
    "https://www.cbs.com",
    "https://www.nbc.com/",
    "https://slashdot.org",
    "https://www.informationweek.com",
    "https://apache.org",
    "https://www.un.org",
};

#define MAX_PARALLEL 10 /* number of simultaneous transfers */
#define NUM_URLS sizeof(urls)/sizeof(char *)

static size_t write_cb(char *data, size_t n, size_t l, void *userp)
{
    /* take care of the data here, ignored in this example */
    (void)data;
    (void)userp;
    return n*l;
}

static void add_transfer(CURLM *cm, int i)
{
    CURL *eh = curl_easy_init();
    curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(eh, CURLOPT_URL, urls[i]);
    curl_easy_setopt(eh, CURLOPT_PRIVATE, urls[i]);
    curl_multi_add_handle(cm, eh);
}

int test()
{

    ///-----------------------------------------------------------
    CURLM *cm;
    CURLMsg *msg;
    unsigned int transfers = 0;
    int msgs_left = -1;
    int still_alive = 1;

    curl_global_init(CURL_GLOBAL_ALL);
    cm = curl_multi_init();

    /* Limit the amount of simultaneous connections curl should allow: */
    curl_multi_setopt(cm, CURLMOPT_MAXCONNECTS, (long)MAX_PARALLEL);

    for(transfers = 0; transfers < MAX_PARALLEL; transfers++)
    {
        add_transfer(cm, transfers);
    }

    do
    {
        curl_multi_perform(cm, &still_alive);

        while((msg = curl_multi_info_read(cm, &msgs_left)))
        {
            if(msg->msg == CURLMSG_DONE)
            {
                char *url;
                CURL *e = msg->easy_handle;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &url);
                fprintf(stderr, "R: %d - %s <%s>\n",
                        msg->data.result, curl_easy_strerror(msg->data.result), url);
                curl_multi_remove_handle(cm, e);
                curl_easy_cleanup(e);
            }
            else
            {
                fprintf(stderr, "E: CURLMsg (%d)\n", msg->msg);
            }
            if(transfers < NUM_URLS)
            {
                add_transfer(cm, transfers++);
            }
        }
        if(still_alive)
        {
            curl_multi_wait(cm, NULL, 0, 1000, NULL);
        }

    }
    while(still_alive || (transfers < NUM_URLS));

    curl_multi_cleanup(cm);
    curl_global_cleanup();

    return EXIT_SUCCESS;
}

static void *pull_one_url(void *url)
{
    CURL *curl;

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_perform(curl); /* ignores error */
    curl_easy_cleanup(curl);

    return NULL;
}

#define NUMT 1


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    //-----------------test();----------------------------
    curl_global_init(CURL_GLOBAL_ALL);
    pthread_t tid[NUMT];
    int i;
    /* Must initialize libcurl before any threads are started */
    curl_global_init(CURL_GLOBAL_ALL);

    for(i = 0; i< NUMT; i++)
    {
        int error = pthread_create(&tid[i],      NULL, /* default attributes please */
                                   pull_one_url, (void *)urls[i]);
        if(0 != error)
        {
            fprintf(stderr, "Couldn't run thread number %d, errno %d\n", i, error);
        }
        else
        {
            fprintf(stderr, "Thread %d, gets %s\n", i, urls[i]);
        }
    }

    /* now wait for all threads to terminate */
    for(i = 0; i< NUMT; i++)
    {
        pthread_join(tid[i], NULL);
        fprintf(stderr, "Thread %d terminated\n", i);
    }
    curl_global_cleanup();
    //--------------------------------------------------------

    return a.exec();

}
