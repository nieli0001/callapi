#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

struct MemoryStruct {
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        /* out of memory! */
        printf("not enough memory (realloc returned NULL)\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

int main(void) {
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0; 

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();

    // 인증키
    char *authkey = "fTBWVRKLRGOkfqrvfYxTfymAF7ZLLtqo";

    // 검색 요청 날짜 (예: "20240101 / 2024-01-01 / default(빈 문자열 값)으로 지정하면 현재일")
    char *searchdate = "20240607"; // 

    // API 타입 환율이라서 "AP01"
    char *data_type = "AP01"; 

    char url[256];
    snprintf(url, sizeof(url), "https://www.koreaexim.go.kr/site/program/financial/exchangeJSON?authkey=%s&searchdate=%s&data=%s", authkey, searchdate, data_type);


    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // SSL 인증서 검증 비활성화(안하면 인증서 오류떠서 그냥 이렇게 해놨어요)
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);


    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
    } else {
        printf("%lu bytes retrieved\n", (unsigned long)chunk.size);
        printf("%s\n", chunk.memory);

        // Json 파싱도 가능한데 라이브러리 이슈로 그냥 패스
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return 0;
}
