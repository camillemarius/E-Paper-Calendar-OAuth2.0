#include "WebUploaderHtml.h"


const char WEB_UPLOADER_HTML[] = R"rawliteral(

<!DOCTYPE html>

<html lang="de">

<head>

<meta charset="UTF-8">

<meta
    name="viewport"
    content="width=device-width, initial-scale=1.0"
>

<title>EPD Image Upload</title>


<style>

*
{
    box-sizing: border-box;
}


body
{
    margin: 0;
    padding: 20px;

    background: #eeeeee;
    color: #222;

    font-family:
        Arial,
        Helvetica,
        sans-serif;
}


.container
{
    width: 100%;
    max-width: 900px;

    margin: 0 auto;
}


.card
{
    background: white;

    padding: 20px;
    margin-bottom: 20px;

    border-radius: 10px;

    box-shadow:
        0 2px 8px rgba(0, 0, 0, 0.10);
}


h1
{
    margin-top: 0;
}


h2
{
    margin-top: 0;
}


.info
{
    font-size: 14px;
    line-height: 1.5;

    color: #555;
}


label
{
    display: block;

    margin-top: 15px;
    margin-bottom: 6px;

    font-weight: bold;
}


select
{
    width: 100%;

    padding: 12px;

    border:
        1px solid #cccccc;

    border-radius: 6px;

    background: #fafafa;

    font-size: 16px;
}


input[type="file"]
{
    width: 100%;

    padding: 10px;

    margin-top: 10px;

    border:
        1px solid #cccccc;

    border-radius: 6px;

    background: #fafafa;
}


button
{
    width: 100%;

    padding: 14px;

    margin-top: 15px;

    border: none;

    border-radius: 6px;

    background: #222;

    color: white;

    font-size: 16px;

    cursor: pointer;
}


button:hover
{
    background: #444;
}


button:disabled
{
    background: #999;

    cursor: not-allowed;
}


canvas
{
    display: block;

    width: 100%;

    max-width: 800px;

    height: auto;

    aspect-ratio: 5 / 3;

    border:
        1px solid #cccccc;

    background: white;
}


.status
{
    margin-top: 15px;

    padding: 10px;

    background: #f5f5f5;

    border-radius: 6px;

    white-space: pre-wrap;

    font-size: 14px;
}

</style>

</head>


<body>


<div class="container">


    <div class="card">

        <h1>EPD Image Upload</h1>


        <div class="info">

            <p>
                Das Bild wird vollständig
                im Browser verarbeitet.
            </p>


            <p>
                Auflösung:
                <b>800 × 480</b>
                <br>

                Format:
                <b>3 Bit/Pixel</b>
                <br>

                Upload:
                <b>192000 Bytes</b>
            </p>

        </div>


        <!-- =====================================================
             Display selection
             ===================================================== -->

        <label for="displayType">
            Display
        </label>


        <select id="displayType">

            <option value="GDEP073E01">
                GDEP073E01 – 7 Farben
            </option>

            <option value="FPC8612">
                FPC8612 – 3 Farben
            </option>

        </select>


        <!-- =====================================================
             File selection
             ===================================================== -->

        <label for="fileInput">
            Bild
        </label>


        <input
            id="fileInput"
            type="file"
            accept="image/png,image/jpeg,image/bmp"
        >


        <button
            id="uploadButton"
            disabled
        >
            Bild übertragen
        </button>


        <div
            id="status"
            class="status"
        >
            Bitte ein Bild auswählen.
        </div>

    </div>


    <!-- =========================================================
         Preview
         ========================================================= -->

    <div class="card">

        <h2>Vorschau</h2>


        <canvas
            id="preview"
            width="800"
            height="480"
        ></canvas>

    </div>


</div>


<script>

"use strict";


/*
 * ============================================================
 * E-Paper resolution
 * ============================================================
 */

const EPD_W = 800;
const EPD_H = 480;


/*
 * ============================================================
 * Packed image size
 *
 * 800 * 480 = 384000 Pixel
 *
 * 2 Pixel pro Byte
 *
 * = 192000 Bytes
 * ============================================================
 */

const EXPECTED_PACKED_SIZE =
    (EPD_W * EPD_H) / 2;


/*
 * ============================================================
 * Selected display
 * ============================================================
 */

let selectedDisplay =
    "GDEP073E01";


/*
 * ============================================================
 * Palettes
 *
 * Index:
 *
 * 0 = Black
 * 1 = White
 * 2 = ...
 * ============================================================
 */


/*
 * 7-color display
 */

const PALETTE_GDEP073E01 =
[
    [0,   0,   0],       // 0 Black
    [255, 255, 255],     // 1 White
    [255, 255, 0],       // 2 Yellow
    [255, 0,   0],       // 3 Red
    [0,   0,   255],     // 4 Blue
    [0,   255, 0],       // 5 Green
    [255, 128, 0]        // 6 Orange
];


/*
 * 3-color display
 *
 * FPC8612
 */

const PALETTE_FPC8612 =
[
    [0,   0,   0],       // 0 Black
    [255, 255, 255],     // 1 White
    [255, 0,   0]        // 2 Red
];


/*
 * ============================================================
 * Return palette for selected display
 * ============================================================
 */

function getPalette()
{
    if (
        selectedDisplay ===
        "FPC8612"
    )
    {
        return PALETTE_FPC8612;
    }


    return PALETTE_GDEP073E01;
}


/*
 * ============================================================
 * Processed image
 * ============================================================
 */

let processedImage =
    null;


/*
 * ============================================================
 * Load image file
 * ============================================================
 */

function loadImage(file)
{
    return new Promise(
        (resolve, reject) =>
        {
            const image =
                new Image();


            image.onload =
                () =>
                {
                    URL.revokeObjectURL(
                        image.src
                    );

                    resolve(image);
                };


            image.onerror =
                () =>
                {
                    URL.revokeObjectURL(
                        image.src
                    );

                    reject(
                        new Error(
                            "Bild konnte nicht geladen werden."
                        )
                    );
                };


            image.src =
                URL.createObjectURL(file);
        }
    );
}


/*
 * ============================================================
 * Rotate portrait image
 *
 * 480 x 800 -> 800 x 480
 * ============================================================
 */

function rotatePortraitImage(image)
{
    const width =
        image.naturalWidth ||
        image.width;


    const height =
        image.naturalHeight ||
        image.height;


    if (
        width !== 480 ||
        height !== 800
    )
    {
        return image;
    }


    const canvas =
        document.createElement(
            "canvas"
        );


    canvas.width = 800;
    canvas.height = 480;


    const ctx =
        canvas.getContext("2d");


    /*
     * 90° counter-clockwise
     */

    ctx.translate(
        0,
        480
    );


    ctx.rotate(
        -Math.PI / 2
    );


    ctx.drawImage(
        image,
        0,
        0,
        480,
        800
    );


    return canvas;
}


/*
 * ============================================================
 * Scale image
 *
 * IMPORTANT:
 *
 * Das Bild wird IMMER auf 800 Pixel Breite skaliert.
 *
 * Dadurch darf das Bild oben/unten abgeschnitten werden.
 *
 * ============================================================
 */

function scaleImage(source)
{
    const sourceWidth =
        source.width ||
        source.naturalWidth;


    const sourceHeight =
        source.height ||
        source.naturalHeight;


    /*
     * Immer auf 800 Pixel Breite skalieren.
     */

    const scale =
        EPD_W / sourceWidth;


    const width =
        EPD_W;


    const height =
        Math.max(
            1,
            Math.round(
                sourceHeight * scale
            )
        );


    const canvas =
        document.createElement(
            "canvas"
        );


    canvas.width =
        width;


    canvas.height =
        height;


    const ctx =
        canvas.getContext("2d");


    ctx.imageSmoothingEnabled =
        true;


    ctx.imageSmoothingQuality =
        "high";


    ctx.drawImage(
        source,

        0,
        0,
        sourceWidth,
        sourceHeight,

        0,
        0,
        width,
        height
    );


    return canvas;
}


/*
 * ============================================================
 * Create final 800 x 480 image
 *
 * Image is scaled to FULL WIDTH.
 *
 * If it is higher than 480:
 *
 *     top/bottom are cropped.
 *
 * ============================================================
 */

function createFinalCanvas(source)
{
    const canvas =
        document.createElement(
            "canvas"
        );


    canvas.width =
        EPD_W;


    canvas.height =
        EPD_H;


    const ctx =
        canvas.getContext("2d");


    /*
     * White background
     */

    ctx.fillStyle =
        "rgb(255, 255, 255)";


    ctx.fillRect(
        0,
        0,
        EPD_W,
        EPD_H
    );


    /*
     * Scale to 800 px width.
     */

    const scaled =
        scaleImage(source);


    /*
     * Horizontal position:
     *
     * always exactly 0
     */

    const x =
        0;


    /*
     * Vertical position:
     *
     * Center image.
     *
     * If image is higher than 480,
     * top and bottom are cropped equally.
     */

    const y =
        Math.floor(
            (EPD_H - scaled.height) / 2
        );


    ctx.imageSmoothingEnabled =
        true;


    ctx.imageSmoothingQuality =
        "high";


    ctx.drawImage(
        scaled,
        x,
        y
    );


    return canvas;
}


/*
 * ============================================================
 * Find nearest palette color
 * ============================================================
 */

function nearestColor(
    r,
    g,
    b
)
{
    const palette =
        getPalette();


    let bestIndex =
        0;


    let bestDistance =
        Number.POSITIVE_INFINITY;


    for (
        let i = 0;
        i < palette.length;
        i++
    )
    {
        const color =
            palette[i];


        const dr =
            r - color[0];


        const dg =
            g - color[1];


        const db =
            b - color[2];


        const distance =
            dr * dr +
            dg * dg +
            db * db;


        if (
            distance <
            bestDistance
        )
        {
            bestDistance =
                distance;


            bestIndex =
                i;
        }
    }


    return bestIndex;
}


/*
 * ============================================================
 * Floyd-Steinberg dithering
 * ============================================================
 */

function ditherImage(canvas)
{
    const ctx =
        canvas.getContext("2d");


    const imageData =
        ctx.getImageData(
            0,
            0,
            EPD_W,
            EPD_H
        );


    const source =
        imageData.data;


    /*
     * RGB floating point buffer
     */

    const pixels =
        new Float32Array(
            EPD_W *
            EPD_H *
            3
        );


    /*
     * Copy RGB
     */

    for (
        let i = 0;
        i < EPD_W * EPD_H;
        i++
    )
    {
        const sourceIndex =
            i * 4;


        const pixelIndex =
            i * 3;


        pixels[pixelIndex + 0] =
            source[sourceIndex + 0];


        pixels[pixelIndex + 1] =
            source[sourceIndex + 1];


        pixels[pixelIndex + 2] =
            source[sourceIndex + 2];
    }


    /*
     * One palette index per pixel
     */

    const indices =
        new Uint8Array(
            EPD_W * EPD_H
        );


    /*
     * Floyd-Steinberg
     */

    for (
        let y = 0;
        y < EPD_H;
        y++
    )
    {
        for (
            let x = 0;
            x < EPD_W;
            x++
        )
        {
            const pixel =
                y * EPD_W + x;


            const dataIndex =
                pixel * 3;


            const r =
                pixels[dataIndex + 0];


            const g =
                pixels[dataIndex + 1];


            const b =
                pixels[dataIndex + 2];


            /*
             * Clamp
             */

            const lookupR =
                Math.max(
                    0,
                    Math.min(255, r)
                );


            const lookupG =
                Math.max(
                    0,
                    Math.min(255, g)
                );


            const lookupB =
                Math.max(
                    0,
                    Math.min(255, b)
                );


            /*
             * Nearest palette color
             */

            const paletteIndex =
                nearestColor(
                    lookupR,
                    lookupG,
                    lookupB
                );


            indices[pixel] =
                paletteIndex;


            const palette =
                getPalette();


            const color =
                palette[
                    paletteIndex
                ];


            /*
             * Quantization error
             */

            const errorR =
                r - color[0];


            const errorG =
                g - color[1];


            const errorB =
                b - color[2];


            /*
             * Right 7/16
             */

            if (
                x + 1 < EPD_W
            )
            {
                const index =
                    (
                        y * EPD_W +
                        x + 1
                    ) * 3;


                pixels[index + 0] +=
                    errorR * 7 / 16;


                pixels[index + 1] +=
                    errorG * 7 / 16;


                pixels[index + 2] +=
                    errorB * 7 / 16;
            }


            /*
             * Bottom-left 3/16
             */

            if (
                x > 0 &&
                y + 1 < EPD_H
            )
            {
                const index =
                    (
                        (y + 1) * EPD_W +
                        x - 1
                    ) * 3;


                pixels[index + 0] +=
                    errorR * 3 / 16;


                pixels[index + 1] +=
                    errorG * 3 / 16;


                pixels[index + 2] +=
                    errorB * 3 / 16;
            }


            /*
             * Bottom 5/16
             */

            if (
                y + 1 < EPD_H
            )
            {
                const index =
                    (
                        (y + 1) * EPD_W +
                        x
                    ) * 3;


                pixels[index + 0] +=
                    errorR * 5 / 16;


                pixels[index + 1] +=
                    errorG * 5 / 16;


                pixels[index + 2] +=
                    errorB * 5 / 16;
            }


            /*
             * Bottom-right 1/16
             */

            if (
                x + 1 < EPD_W &&
                y + 1 < EPD_H
            )
            {
                const index =
                    (
                        (y + 1) * EPD_W +
                        x + 1
                    ) * 3;


                pixels[index + 0] +=
                    errorR * 1 / 16;


                pixels[index + 1] +=
                    errorG * 1 / 16;


                pixels[index + 2] +=
                    errorB * 1 / 16;
            }
        }
    }


    return indices;
}


/*
 * ============================================================
 * Pack image
 *
 * Two 3-bit palette indices per byte.
 *
 * first  = bits 7..3
 * second = bits 2..0
 * ============================================================
 */

function packImage(indices)
{
    const pixelCount =
        EPD_W * EPD_H;


    const byteCount =
        pixelCount / 2;


    const packed =
        new Uint8Array(
            byteCount
        );


    for (
        let i = 0;
        i < pixelCount;
        i += 2
    )
    {
        const first =
            indices[i] & 0x07;


        const second =
            indices[i + 1] & 0x07;


        packed[i / 2] =
            (first << 3) |
            second;
    }


    return packed;
}


/*
 * ============================================================
 * Draw packed preview
 *
 * Shows exactly what is uploaded.
 * ============================================================
 */

function drawPackedPreview(packed)
{
    const canvas =
        document.getElementById(
            "preview"
        );


    const ctx =
        canvas.getContext("2d");


    const imageData =
        ctx.createImageData(
            EPD_W,
            EPD_H
        );


    const output =
        imageData.data;


    const pixelCount =
        EPD_W * EPD_H;


    const palette =
        getPalette();


    for (
        let i = 0;
        i < pixelCount;
        i++
    )
    {
        const packedIndex =
            Math.floor(i / 2);


        const value =
            packed[packedIndex];


        let paletteIndex;


        /*
         * First pixel
         */

        if (
            (i & 1) === 0
        )
        {
            paletteIndex =
                (value >> 3) & 0x07;
        }


        /*
         * Second pixel
         */

        else
        {
            paletteIndex =
                value & 0x07;
        }


        /*
         * Safety fallback
         */

        if (
            paletteIndex >=
            palette.length
        )
        {
            paletteIndex =
                1;
        }


        const color =
            palette[
                paletteIndex
            ];


        const outputIndex =
            i * 4;


        output[outputIndex + 0] =
            color[0];


        output[outputIndex + 1] =
            color[1];


        output[outputIndex + 2] =
            color[2];


        output[outputIndex + 3] =
            255;
    }


    ctx.putImageData(
        imageData,
        0,
        0
    );
}


/*
 * ============================================================
 * Complete image processing
 * ============================================================
 */

async function processImage(file)
{
    const status =
        document.getElementById(
            "status"
        );


    status.textContent =
        "Bild wird geladen...";


    let image =
        await loadImage(file);


    /*
     * Portrait rotation
     */

    image =
        rotatePortraitImage(
            image
        );


    status.textContent =
        "Bild wird auf volle Breite skaliert...";


    /*
     * Create final 800 x 480
     */

    const finalCanvas =
        createFinalCanvas(
            image
        );


    status.textContent =
        "Floyd-Steinberg Dithering läuft...";


    /*
     * Let browser update status
     */

    await new Promise(
        resolve =>
            setTimeout(
                resolve,
                10
            )
    );


    const indices =
        ditherImage(
            finalCanvas
        );


    status.textContent =
        "Bilddaten werden gepackt...";


    const packed =
        packImage(
            indices
        );


    /*
     * Verify size
     */

    if (
        packed.length !==
        EXPECTED_PACKED_SIZE
    )
    {
        throw new Error(
            "Interner Fehler: falsche Datenmenge."
        );
    }


    /*
     * Draw exact packed preview
     */

    drawPackedPreview(
        packed
    );


    const palette =
        getPalette();


    status.textContent =
        "Bild fertig verarbeitet.\n" +
        "Display: " +
        selectedDisplay +
        "\n" +
        "Farben: " +
        palette.length +
        "\n" +
        "Daten: " +
        packed.length +
        " Bytes.";


    return packed;
}


/*
 * ============================================================
 * Upload
 * ============================================================
 */

async function uploadImage(packedImage)
{
    const status =
        document.getElementById(
            "status"
        );


    if (
        !packedImage
    )
    {
        status.textContent =
            "Kein verarbeitetes Bild vorhanden.";

        return;
    }


    if (
        packedImage.length !==
        EXPECTED_PACKED_SIZE
    )
    {
        status.textContent =
            "Fehler: Falsche Bildgröße: " +
            packedImage.length +
            " Bytes.";

        return;
    }


    console.log(
        "Upload gestartet"
    );


    console.log(
        "Display:",
        selectedDisplay
    );


    console.log(
        "Upload-Größe:",
        packedImage.length,
        "Bytes"
    );


    status.textContent =
        "Bild wird übertragen...\n" +
        packedImage.length +
        " Bytes";


    try
    {
        const response =
            await fetch(
                "/upload",
                {
                    method: "POST",

                    headers:
                    {
                        "Content-Type":
                            "application/octet-stream"
                    },

                    body:
                        packedImage
                }
            );


        console.log(
            "HTTP Status:",
            response.status
        );


        const text =
            await response.text();


        console.log(
            "ESP32 Antwort:",
            text
        );


        if (
            !response.ok
        )
        {
            throw new Error(
                "HTTP " +
                response.status +
                ": " +
                text
            );
        }


        status.textContent =
            "Upload erfolgreich!\n" +
            "ESP32: " +
            text;


        console.log(
            "Upload erfolgreich"
        );
    }
    catch (error)
    {
        console.error(
            "Upload Fehler:",
            error
        );


        status.textContent =
            "Fehler beim Upload:\n" +
            error.message;


        alert(
            "Fehler beim Upload:\n" +
            error.message
        );
    }
}


/*
 * ============================================================
 * Display selection
 * ============================================================
 */

document
    .getElementById(
        "displayType"
    )
    .addEventListener(
        "change",
        async event =>
        {
            selectedDisplay =
                event.target.value;


            /*
             * If an image is already selected,
             * process it again immediately.
             */

            const fileInput =
                document.getElementById(
                    "fileInput"
                );


            const file =
                fileInput.files[0];


            if (
                !file
            )
            {
                return;
            }


            const button =
                document.getElementById(
                    "uploadButton"
                );


            button.disabled =
                true;


            try
            {
                processedImage =
                    await processImage(
                        file
                    );


                button.disabled =
                    false;
            }
            catch (error)
            {
                processedImage =
                    null;


                button.disabled =
                    true;


                document
                    .getElementById(
                        "status"
                    )
                    .textContent =
                        "Fehler:\n" +
                        error.message;
            }
        }
    );


/*
 * ============================================================
 * File selection
 * ============================================================
 */

document
    .getElementById(
        "fileInput"
    )
    .addEventListener(
        "change",
        async event =>
        {
            const file =
                event.target.files[0];


            if (!file)
            {
                processedImage =
                    null;


                document
                    .getElementById(
                        "uploadButton"
                    )
                    .disabled =
                        true;


                document
                    .getElementById(
                        "status"
                    )
                    .textContent =
                        "Bitte ein Bild auswählen.";


                return;
            }


            const button =
                document.getElementById(
                    "uploadButton"
                );


            button.disabled =
                true;


            try
            {
                processedImage =
                    await processImage(
                        file
                    );


                button.disabled =
                    false;
            }
            catch (error)
            {
                processedImage =
                    null;


                button.disabled =
                    true;


                document
                    .getElementById(
                        "status"
                    )
                    .textContent =
                        "Fehler:\n" +
                        error.message;


                console.error(
                    "Bildverarbeitung fehlgeschlagen:",
                    error
                );
            }
        }
    );


/*
 * ============================================================
 * Upload button
 * ============================================================
 */

document
    .getElementById(
        "uploadButton"
    )
    .addEventListener(
        "click",
        async () =>
        {
            if (
                processedImage === null
            )
            {
                return;
            }


            await uploadImage(
                processedImage
            );
        }
    );

</script>


</body>

</html>

)rawliteral";
