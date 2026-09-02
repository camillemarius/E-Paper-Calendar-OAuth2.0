#pragma once

#include <Arduino.h>
#include <WebServer.h>
#include <externalFlash.h>
#include "ImagePalette.h"

class WebUploader
{
public:
    using UploadCallback = void (*)(externalFlash& flash, size_t len, ImagePalette palette);

    WebUploader(WebServer& server, externalFlash& flash, ImagePalette palette);

    void setUploadCallback(UploadCallback cb);
    void begin();

private:
    static constexpr size_t EXPECTED_IMAGE_SIZE = 192000;

    WebServer& server;
    externalFlash& flash;
    ImagePalette paletteMode;
    UploadCallback callback = nullptr;

    size_t imageSize = 0;
    bool uploadError = false;

    const char* getHtml();
    void handleUpload();

};
