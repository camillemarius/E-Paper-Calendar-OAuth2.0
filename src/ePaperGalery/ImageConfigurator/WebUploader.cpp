
#include "WebUploader.h"
#include "WebUploaderHtml.h"
#include <logger.h>

WebUploader::WebUploader(WebServer& server, externalFlash& flash, ImagePalette palette, const char* displayId)
    : server(server), flash(flash), paletteMode(palette), displayId(displayId)
{
}

void WebUploader::setUploadCallback(UploadCallback cb)
{
    callback = cb;
}

void WebUploader::begin()
{
    server.on("/", HTTP_GET, [this]()
    {
        server.send(200, "text/html; charset=utf-8", getHtml());
    });

    server.on("/favicon.ico", HTTP_GET, [this]()
    {
        server.send(204);
    });

    // Display und Farbanzahl, für die diese Firmware gebaut ist
    server.on("/info", HTTP_GET, [this]()
    {
        String json = String("{\"display\":\"") + displayId +
                      "\",\"colors\":" + static_cast<int>(paletteMode) + "}";
        server.send(200, "application/json", json);
    });

    server.on("/upload", HTTP_POST,
        [this]()
        {
            handleUpload();
        },
        [this]()
        {
            HTTPRaw& raw = server.raw();

            if (raw.status == RAW_START)
            {
                imageSize = 0;
                uploadError = false;
                LOG_DEBUG("EPD Upload START");

                // Bildbereich löschen: ein NOR-Flash lässt sich nur auf gelöschte Bytes schreiben
                if (!flash.eraseArea(0, EXPECTED_IMAGE_SIZE))
                {
                    LOG_ERROR("EPD Upload ERROR: Flash erase failed!");
                    uploadError = true;
                }
            }
            else if (raw.status == RAW_WRITE)
            {
                if (raw.currentSize == 0 || uploadError)
                    return;

                if (imageSize + raw.currentSize > EXPECTED_IMAGE_SIZE)
                {
                    LOG_ERROR("EPD Upload ERROR: zu viele Daten");
                    uploadError = true;
                    return;
                }

                if (!flash.writeImage(imageSize, raw.buf, raw.currentSize))
                {
                    LOG_ERROR("EPD Upload ERROR: Flash write failed!");
                    uploadError = true;
                    return;
                }

                imageSize += raw.currentSize;
            }
            else if (raw.status == RAW_END)
            {
                LOG_DEBUG("EPD Upload END: %u Bytes", (unsigned)imageSize);
            }
            else if (raw.status == RAW_ABORTED)
            {
                LOG_ERROR("EPD Upload ABORTED");
                imageSize = 0;
                uploadError = true;
            }
        }
    );

    server.onNotFound([this]()
    {
        server.send(404, "text/plain; charset=utf-8", "Nicht gefunden!");
    });
}

const char* WebUploader::getHtml()
{
    return WEB_UPLOADER_HTML;
}

void WebUploader::handleUpload()
{
    if (uploadError)
    {
        server.send(500, "text/plain; charset=utf-8", "Fehler beim Schreiben in den Flash!");
        imageSize = 0;
        uploadError = false;
        return;
    }

    if (imageSize != EXPECTED_IMAGE_SIZE)
    {
        LOG_ERROR("EPD Upload ERROR: %u statt %u Bytes",
                  (unsigned)imageSize,
                  (unsigned)EXPECTED_IMAGE_SIZE);

        server.send(400, "text/plain; charset=utf-8", "Fehler: Falsche Bilddaten!");
        imageSize = 0;
        return;
    }

    LOG_DEBUG("EPD Upload OK: %u Bytes", (unsigned)imageSize);

    if (callback != nullptr)
        callback(flash, imageSize, paletteMode);
    else
        LOG_DEBUG("WARNUNG: Kein Upload-Callback gesetzt!");

    server.send(200, "text/plain; charset=utf-8", "Bild erfolgreich uebertragen!");

    imageSize = 0;
}
