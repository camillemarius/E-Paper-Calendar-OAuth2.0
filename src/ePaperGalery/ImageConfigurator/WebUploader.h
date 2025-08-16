#pragma once
#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>
#include <WebServer.h>
#include <logger.h>
#include <TJpg_Decoder.h>
#include <vector>

class WebUploader {
public:
    // Callback für JPEG-Daten
    typedef void (*UploadCallback)(uint8_t* jpegData, size_t len);

    WebUploader(WebServer& srv) : server(srv), callback(nullptr) {}

    void setUploadCallback(UploadCallback cb) { callback = cb; }

    void begin() {
        // HTML Upload-Form
        server.on("/", HTTP_GET, [this]() {
            server.send(200, "text/html",
                "<form method='POST' action='/upload' enctype='multipart/form-data'>"
                "<input type='file' name='img'><input type='submit' value='Upload'>"
                "</form>");
        });

        // Upload-Handler
        server.on("/upload", HTTP_POST, [this]() {
            server.send(200, "text/plain", "Upload fertig!");
        }, [this]() { handleUpload(); });

        // 404 Handler
        server.onNotFound([this]() {
            Serial.printf("Unbekannte URL: %s\n", server.uri().c_str());
            server.send(404, "text/plain", "Nicht gefunden!");
        });
    }

    // JPEG direkt dekodieren und auf Display zeigen
    template<typename DisplayType>
    void decodeAndShowJPEG(DisplayType& display, const uint8_t* jpegData, size_t len) {
        TJpgDec.setCallback([](int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap) -> bool {
            // Hier Bitmap auf Display zeichnen
            // display.drawBitmap(x, y, bitmap, w, h); 
            return true; // Weiter dekodieren
        });

        // Statt setJpgBuffer + decode:
        TJpgDec.drawJpg(0, 0, jpegData, len);
    }

private:
    WebServer& server;
    UploadCallback callback;

    void handleUpload() {
        static std::vector<uint8_t> buf;
        HTTPUpload& upload = server.upload();

        if (upload.status == UPLOAD_FILE_START) {
            buf.clear();
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            buf.insert(buf.end(), upload.buf, upload.buf + upload.currentSize);
        } else if (upload.status == UPLOAD_FILE_END) {
            LOG_DEBUG("Upload fertig, Größe: %d Bytes", buf.size());
            if (callback) callback(buf.data(), buf.size());
        }
    }
};
