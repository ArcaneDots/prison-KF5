/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: MIT
*/

#include "format_p.h"
#include "scanresult_p.h"
#include "videoscannerframe_p.h"
#include "videoscannerworker_p.h"

#include <QDebug>
#include <QImage>
#include <QTransform>

#include <ZXing/ReadBarcode.h>
#include <ZXing/TextUtfEncoding.h>

using namespace Prison;

VideoScannerWorker::VideoScannerWorker(QObject *parent)
    : QObject(parent)
{
    connect(this, &VideoScannerWorker::scanFrameRequest, this, &VideoScannerWorker::slotScanFrame, Qt::QueuedConnection);
}

void VideoScannerWorker::slotScanFrame(VideoScannerFrame frame)
{
    ZXing::Result zxRes(ZXing::DecodeStatus::FormatError);
    ZXing::DecodeHints hints;
    hints.setFormats(frame.formats() == Format::NoFormat ? ZXing::BarcodeFormats::all() : Format::toZXing(frame.formats()));

    frame.map();
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    switch (frame.pixelFormat()) {
    case QVideoFrame::Format_Invalid: // already checked before we get here
    case QVideoFrame::NPixelFormats: // just to silence the unhandled case warning
        break;

    // formats ZXing can consume directly
    case QVideoFrame::Format_ARGB32:
    case QVideoFrame::Format_ARGB32_Premultiplied:
    case QVideoFrame::Format_RGB32:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::XRGB}, hints);
        break;
    case QVideoFrame::Format_RGB24:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::RGB}, hints);
        break;
    case QVideoFrame::Format_BGRA32:
    case QVideoFrame::Format_BGRA32_Premultiplied:
    case QVideoFrame::Format_BGR32:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::BGRX}, hints);
        break;
    case QVideoFrame::Format_ABGR32:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::XBGR}, hints);
        break;
    case QVideoFrame::Format_BGR24:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::BGR}, hints);
        break;
    case QVideoFrame::Format_AYUV444:
    case QVideoFrame::Format_AYUV444_Premultiplied:
        zxRes = ZXing::ReadBarcode({frame.bits() + 1, frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 4}, hints);
        break;
    case QVideoFrame::Format_YUV444:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 3}, hints);
        break;
    case QVideoFrame::Format_YUV420P:
    case QVideoFrame::Format_YUV422P:
    case QVideoFrame::Format_YV12:
    case QVideoFrame::Format_NV12:
    case QVideoFrame::Format_NV21:
    case QVideoFrame::Format_IMC1:
    case QVideoFrame::Format_IMC2:
    case QVideoFrame::Format_IMC3:
    case QVideoFrame::Format_IMC4:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum}, hints);
        break;
    case QVideoFrame::Format_UYVY:
        zxRes = ZXing::ReadBarcode({frame.bits() + 1, frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 2}, hints);
        break;
    case QVideoFrame::Format_YUYV:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 2}, hints);
        break;
    case QVideoFrame::Format_Y8:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum}, hints);
        break;
    case QVideoFrame::Format_Y16:
        zxRes = ZXing::ReadBarcode({frame.bits() + 1, frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 1}, hints);
        break;

    // formats needing conversion before ZXing can consume them
    case QVideoFrame::Format_RGB565:
    case QVideoFrame::Format_RGB555:
    case QVideoFrame::Format_ARGB8565_Premultiplied:
    case QVideoFrame::Format_BGR565:
    case QVideoFrame::Format_BGR555:
    case QVideoFrame::Format_BGRA5658_Premultiplied:
    case QVideoFrame::Format_Jpeg:
    case QVideoFrame::Format_CameraRaw:
    case QVideoFrame::Format_AdobeDng:
    case QVideoFrame::Format_User:
        frame.convertToImage();
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum}, hints);
        break;
    }
#else
    switch (frame.pixelFormat()) {
    case QVideoFrameFormat::Format_Invalid: // already checked before we get here
        break;
    // formats ZXing can consume directly
    case QVideoFrameFormat::Format_ARGB8888:
    case QVideoFrameFormat::Format_ARGB8888_Premultiplied:
    case QVideoFrameFormat::Format_XRGB8888:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::XRGB}, hints);
        break;
    case QVideoFrameFormat::Format_BGRA8888:
    case QVideoFrameFormat::Format_BGRA8888_Premultiplied:
    case QVideoFrameFormat::Format_BGRX8888:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::BGRX}, hints);
        break;
    case QVideoFrameFormat::Format_ABGR8888:
    case QVideoFrameFormat::Format_XBGR8888:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::XBGR}, hints);
        break;
    case QVideoFrameFormat::Format_RGBA8888:
    case QVideoFrameFormat::Format_RGBX8888:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::RGBX}, hints);
        break;
    case QVideoFrameFormat::Format_AYUV:
    case QVideoFrameFormat::Format_AYUV_Premultiplied:
        zxRes = ZXing::ReadBarcode({frame.bits() + 1, frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 4}, hints);
        break;
    case QVideoFrameFormat::Format_YUV420P:
    case QVideoFrameFormat::Format_YUV422P:
    case QVideoFrameFormat::Format_YV12:
    case QVideoFrameFormat::Format_NV12:
    case QVideoFrameFormat::Format_NV21:
    case QVideoFrameFormat::Format_IMC1:
    case QVideoFrameFormat::Format_IMC2:
    case QVideoFrameFormat::Format_IMC3:
    case QVideoFrameFormat::Format_IMC4:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum}, hints);
        break;
    case QVideoFrameFormat::Format_UYVY:
        zxRes = ZXing::ReadBarcode({frame.bits() + 1, frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 2}, hints);
        break;
    case QVideoFrameFormat::Format_YUYV:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 2}, hints);
        break;
    case QVideoFrameFormat::Format_Y8:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum}, hints);
        break;
    case QVideoFrameFormat::Format_Y16:
        zxRes = ZXing::ReadBarcode({frame.bits() + 1, frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 1}, hints);
        break;
    case QVideoFrameFormat::Format_P010:
    case QVideoFrameFormat::Format_P016:
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum, 0, 1}, hints);
        break;

    // formats needing conversion before ZXing can consume them
    case QVideoFrameFormat::Format_Jpeg:
    case QVideoFrameFormat::Format_SamplerExternalOES:
    case QVideoFrameFormat::Format_SamplerRect:
        frame.convertToImage();
        zxRes = ZXing::ReadBarcode({frame.bits(), frame.width(), frame.height(), ZXing::ImageFormat::Lum}, hints);
        break;
    }
#endif
    frame.unmap();

    // process scan result
    ScanResult scanResult;
    if (zxRes.isValid()) {
        auto res = ScanResultPrivate::get(scanResult);

        // distinguish between binary and text content
        const auto hasWideChars = std::any_of(zxRes.text().begin(), zxRes.text().end(), [](auto c) {
            return c > 255;
        });
        const auto hasControlChars = std::any_of(zxRes.text().begin(), zxRes.text().end(), [](auto c) {
            return c < 20 && c != 0x0a && c != 0x0d;
        });
        if (hasWideChars || !hasControlChars) {
            res->content = QString::fromStdString(ZXing::TextUtfEncoding::ToUtf8(zxRes.text()));
        } else {
            QByteArray b;
            b.resize(zxRes.text().size());
            std::copy(zxRes.text().begin(), zxRes.text().end(), b.begin());
            res->content = b;
        }

        // determine the bounding rect
        // the cooridinates we get from ZXing are a polygon, we need to determine the
        // bounding rect manually from its coordinates
        const auto p = zxRes.position();
        int x1 = std::numeric_limits<int>::max();
        int y1 = std::numeric_limits<int>::max();
        int x2 = std::numeric_limits<int>::min();
        int y2 = std::numeric_limits<int>::min();
        for (int i = 0; i < 4; ++i) {
            x1 = std::min(x1, p[i].x);
            y1 = std::min(y1, p[i].y);
            x2 = std::max(x2, p[i].x);
            y2 = std::max(y2, p[i].y);
        }
        res->boundingRect = QRect(QPoint(x1, y1), QPoint(x2, y2));

        // apply frame transformations to the bounding rect
        if (frame.isVerticallyFlipped()) {
            QTransform t;
            t.scale(1, -1);
            t.translate(0, -frame.height());
            res->boundingRect = t.mapRect(res->boundingRect);
        }

        res->format = Format::toFormat(zxRes.format());
    }

    Q_EMIT result(scanResult);
}

void VideoScannerThread::run()
{
    exec();
}

#include "moc_videoscannerworker_p.cpp"